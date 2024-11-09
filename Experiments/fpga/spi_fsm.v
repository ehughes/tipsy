module spi_fsm(
    input wire clk,
    input wire rst,
    input wire[7:0] data_in,
    input wire data_ready_in,
    output [7:0] data_out,
    output [15:0] mem_addr,
    output [7:0] mem_data,
    output mem_we
    );

localparam STATE_Init = 8'd0,
           STATE_CmdWriteAddr = 8'd1,
           STATE_CmdWriteLength = 8'd2,
           STATE_CmdWriteData = 8'd3;

reg[7:0] current_state = STATE_Init;
reg[7:0] next_state;
reg[15:0] addr = 16'h0000;
reg[15:0] length = 16'h0000;
reg[15:0] cp = 16'h0000;
reg load_addr = 0;
reg load_length = 0;
reg reset_cp = 0;
reg incr_cp = 0;
reg we_out;

assign mem_addr = addr + cp;
assign mem_data = data_in;
assign mem_we = we_out;
assign data_out = current_state; // use cp[7:0]

// make sure data_ready only fires on one clock
reg [2:0] sync_data_ready;
always@(posedge clk) begin
    sync_data_ready <= {sync_data_ready[1:0], data_ready_in};
end
assign data_ready = sync_data_ready[2:1] == 2'b10;

// address reg
always@(posedge clk) begin
    if (rst) begin
        addr <= 16'h0000;
    end else begin
        if (load_addr && cp == 0) begin
            addr[15:8] <= data_in;
        end
        if (load_addr && cp == 1) begin
            addr[7:0] <= data_in;
        end
    end
end

// length reg
always@(posedge clk) begin
    if (rst) begin
        length <= 16'h0000;
    end else begin
        if (load_length && cp == 0) begin
            length[15:8] <= data_in;
        end
        if (load_length && cp == 1) begin
            length[7:0] <= data_in;
        end
    end
end

// cp counter
always@(posedge clk) begin
    if (rst) begin
        cp <= 16'h0000;
    end else begin
        if (reset_cp) begin
            cp <= 0;
        end
        if (incr_cp) begin
            cp <= cp + 1;
        end
    end
end

// spi cmd state machine
always@(posedge clk) begin
    if (rst) begin
        current_state <= STATE_Init;
    end else begin
        current_state <= next_state;
    end
end

always@(*) begin
    next_state = current_state;
    load_addr = 0;
    load_length = 0;
    reset_cp = 0;
    incr_cp = 0;
    we_out = 0; 
    if (data_ready) begin
        case(current_state)
            STATE_Init: begin
                // command starts with ascii 'w'
                if (data_in == 8'h77) begin
                    next_state = STATE_CmdWriteAddr;
                    reset_cp = 1;
                end
            end
            STATE_CmdWriteAddr: begin
                load_addr = 1;
                if (cp == 0) begin
                    next_state = STATE_CmdWriteAddr;
                    incr_cp = 1;
                end else begin
                    next_state = STATE_CmdWriteLength;
                    reset_cp = 1;
                end
            end
            STATE_CmdWriteLength: begin
                load_length = 1;
                if (cp == 0) begin
                    next_state = STATE_CmdWriteLength;
                    incr_cp = 1;
                end else begin
                    next_state = STATE_CmdWriteData;
                    reset_cp = 1;
                end            
            end
            STATE_CmdWriteData: begin
                we_out = 1;
                if (length == 0 || cp >= length - 1) begin
                    next_state = STATE_Init;
                    reset_cp = 1;
                end else begin
                    incr_cp = 1;
                end
            end
        endcase
    end
end

endmodule
