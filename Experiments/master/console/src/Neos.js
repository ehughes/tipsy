//@module

var Neos = function(log) {
  this.log = log;
  this.log('NEOS HERE');
  this.nodes = {};
  this.start_ms = Date.now();
  this.screamo = "test-node15d46";
  this.stompo = "test-node20af1";
  this.spinno = "test-nodef4db91";
  this.audioo = "test-node20cc2";
  this.winnerDetected = true;
  this.sendCountdown = -1;
  this.winCountdown = -1;
  this.players = [{
    color: 0
  }, {
    color: 32
  }, {
    color: 64
  }, {
    color: 96
  }, {
    color: 128
  }, {
    color: 160
  }, {
    color: 192
  },  {
    color: 224
  }];
  this.round_list = [];
  this.round = 0;
};

/*
Neos.prototype = Object.create(Object.prototype, {
  // foo is a regular 'value property'
  foo: { writable: true, configurable: true, value: 'hello' },
  // bar is a getter-and-setter (accessor) property
  bar: {
    configurable: false,
    get: function() { return 10; },
    set: function(value) { this.console('Setting o.bar to' + value); }
  }
});
*/

function rand(min, max) {
  return Math.floor((Math.random() * (max - min)) + min);
}

function getPlayerList() {
  var list = [];
  for (var i=0; i<8; i++) {
    while (true) {
      var candidate = rand(0, 8);
      var unique = true;
      list.forEach((p) => {
        if (candidate == p) {
          unique = false;
        }
      });
      if (unique) {
        list.push(candidate);
        break;
      }
    }
  }
  return list;
}

Neos.prototype._sendGameConfigs = function () {
  for (var i=0; i<8; i++) {
    this.players[i].score = 0;
  }
  this.round_list = getPlayerList();
  var list = this.round_list;
  this.log("new round: ", {r: this.round, l: list});

  this.send(this.screamo, {
    type: 'game_config',
    mode: 'StartGame',
    players: [
      this.players[list[0]],
      this.players[list[1]]
    ]
  });
  this.send(this.spinno, {
    type: 'game_config',
    mode: 'StartGame',
    players: [
      this.players[list[2]],
      this.players[list[3]]
    ]
  });
  this.send(this.stompo, {
    type: 'game_config',
    mode: 'StartGame',
    players: [
      this.players[list[4]],
      this.players[list[5]],
      this.players[list[6]],
      this.players[list[7]]
    ]
  });
  this.winCountdown = 10;
};

Neos.prototype._updateMasterScore = function (name, winner) {
  var list = this.round_list;
  var pnum;

  if (name == this.screamo) {
    pnum = list[winner];
  }
  if (name == this.spinno) {
    pnum = list[winner+2];
  }
  if (name == this.stompo) {
    pnum = list[winner+4];
  }
  this.players[pnum].wins++;
  this.log("PLAYER %d WINS ROUND %d", [pnum, this.round]);
  this.round++;
  this.log("PLAYERS: ", {p: this.players});

  if (this.round >= 3) {
    var winner;
    var max_wins = 0;
    for (var i=0; i<8; i++) {
      if (this.players[i].wins > max_wins) {
        max_wins = this.players[i].wins;
        winner = i;
      }
    }
    // GAME OVER
    this.log("PLAYER %d WINS IT ALL", [winner]);
    Object.keys(this.nodes).forEach((name) => {
      this.send(name, {
        type: 'game_config',
        mode: 'ShowFlag',
        players: [
          this.players[pnum]
        ]
      });
    });
    this.send(this.audioo, {
      type: 'game_config',
      mode: 'KillAudio',
      state: 'LOP.WAV'
    });
  }
};

Neos.prototype.browser = function(browser) {
  this._browser = browser;
};

Neos.prototype.socket = function(socket) {
  this._socket = socket;
};

Neos.prototype.tick = function() {
  // this.log("tick %d", [Date.now()]);
  Object.keys(this.nodes).forEach((name) => {
    if (!this.nodes[name].last_ms) {
      this.nodes[name].last_ms = Date.now();
    }
    if (Date.now() - this.nodes[name].last_ms > 1200) {
      this.log("%s is stale", [name]);
    }
    if (!this.nodes[name].connected) {
      this.nodes[name].connected = true;
      this.nodes[name].ws = this._socketConnect(name);
    }
  });

  if (this.sendCountdown == 0) {
    this.sendCountdown = -1;
    this._sendGameConfigs();
  }
  if (this.sendCountdown > 0) {
    // this.log("SEND COUNT %d", [this.sendCountdown]);
    this.sendCountdown--;
  }

  if (this.winCountdown == 0) {
    this.winCountdown = -1;
    this.winnerDetected = false;
  }
  if (this.winCountdown > 0) {
    // this.log("WIN COUNT %d", [this.winCountdown]);
    this.winCountdown--;
  }
};

Neos.prototype.start = function() {
  this._browser.start((service) => {
    this.log("mdns cb: ", service);
    if (service.port != 8000) return;
    if (this.nodes[service.name]) return;
    this.log("adding %s", [service.name]);
    service.connected = false;
    this.nodes[service.name] = service;
  });
};

Neos.prototype.playGame = function(config) {
  this.players.forEach((p) => {
    p.wins = 0;
    p.score = 0;
    // this.log(p);
  });
  this.round = 0;
  // Object.keys(this.nodes).forEach((name) => {
  //   this.send(name, {
  //     type: 'game_config',
  //     mode: 'ShowFlag',
  //     players: [
  //       this.players[winner]
  //     ]
  //   });
  // });
  this.send(this.audioo, {
    type: 'game_config',
    mode: 'PlayAudio',
    // mode: 'KillAudio',
    state: 'LOP.WAV'
  });
  this._sendGameConfigs();
};

// resend/confirm?
Neos.prototype.send = function(dst, data) {
  var ws = this.nodes[dst].ws;
  if (!ws) {
    this.log("ws null: %s", dst);
    this.nodes[dst].connected = false;
    return;
  }
  try {
    this._socket.send(ws, JSON.stringify(data));
  }
  catch (e) {
    this.log("ws send failed", dst);
    this.nodes[dst].connected = false;
  }
};

Neos.prototype._socketConnect = function(name) {
  var service = this.nodes[name];
  return this._socket.connect('ws://' + service.ip + ':8000', service.name, (error, name, data) => {
    if (error) {
      this.log("error %s", [error]);
      service.connected = false;
      return;
    }
    this._messageReceived(name, data);
  });
};

Neos.prototype._messageReceived = function(name, data) {
  // this.log("message received, " + name + ":", data);
  this.nodes[name].last_ms = Date.now();
  if (data.winner != -1 && !this.winnerDetected && name != this.audioo) {
    this.winnerDetected = true;
    // send stop game message
    Object.keys(this.nodes).forEach((n) => {
      // skip winning node
      if (name == n) {
        return;
      }
      this.send(n, {
        type: 'game_config',
        mode: 'StopGame'
      });
    });
    this._updateMasterScore(name, data.winner);
    if (this.round < 3) {
      this.sendCountdown = 40;
    }
  }
};

module.exports = Neos;
