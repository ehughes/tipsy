module top(
    // 8MHz clock input
    input uclk,
    // Input from reset button (active low)
    //input rst_n,
    // Outputs to the 8 onboard LEDs
    output [7:0] led,
    // iMX6 SPI connections
    output spi_miso,
    input spi_ss,
    input spi_mosi,
    input spi_sck,
    // LED matrix connections
    output matrix_clk,
    output [2:0] matrix_rgb1,
    output [2:0] matrix_rgb2,
    output [3:0] matrix_addr,
    output matrix_lat,
    output matrix_oe
  );
   
  //wire rst = ~rst_n; // make reset active high
  wire rst = 0;

  // these signals should be high-z when not used
  // assign spi_miso = 1'bz;

  wire done, we;
  wire [7:0] spi_out, spi_in;
  reg [7:0] din = 8'hAA;
  wire [15:0] addr1;
  wire [7:0] data1;
  reg [7:0] led_out = 8'h00;
  assign led = led_out;
  
  //reg[31:0] divcount = 32'h00000000;
  //reg[3:0] count = 4'h0;
  //reg [14:0] addr2;
  //wire [17:0] data2;
  //reg [17:0] video_data = 18'b011111111000000000;
  wire [17:0] video_data;
  wire [9:0] video_addr;
  // always@(posedge uclk) begin
  //     if (divcount == 8000000) begin
  //       divcount <= 0;
  //       count <= count + 1;
  //     end else begin
  //       divcount <= divcount + 1;
  //     end
  //     video_data <= {3'b000,video_addr[5:0],3'b000,video_addr[2:0],3'b000};
  // end
  // always@(posedge uclk) begin
  //   addr2 <= {11'd0, count};
  //   led_out <= data2[7:0];
  // end

  spi_slave spi_bus (
    .clk(uclk),
    .rst(rst),
    .ss(spi_ss),
    .mosi(spi_mosi),
    .miso(spi_miso),
    .sck(spi_sck),
    .done(done),
    .din(din),
    .dout(spi_out)
  );

  spi_fsm spi_handler (
    .clk(uclk),
    .rst(rst),
    .data_in(spi_out),
    .data_ready_in(done),
    .data_out(spi_in),
    .mem_addr(addr1),
    .mem_data(data1),
    .mem_we(we)
  );

  mem videoram (
    .clk(uclk),
    .addr1(addr1),
    .data1(data1),
    .we(we),
    .addr2(video_addr),
    .data2(video_data)
  );

  ledctrl videomatrix (
    .clk(uclk),
    .rst(rst),
    .clk_out(matrix_clk),
    .rgb1(matrix_rgb1),
    .rgb2(matrix_rgb2),
    .led_addr(matrix_addr),
    .lat(matrix_lat),
    .oe(matrix_oe),
    .addr(video_addr),
    .data(video_data)
  );
endmodule