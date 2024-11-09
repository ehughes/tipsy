`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    11:22:18 06/30/2015 
// Design Name: 
// Module Name:    mem 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module mem(
    input [15:0] addr1,
    input [9:0] addr2,
    input we,
    input [7:0] data1,
    output [17:0] data2,
    input clk
    );

   parameter RAM_WIDTH = 8;
   parameter RAM_ADDR_BITS = 10;

   (* RAM_STYLE="AUTO" *)
   reg [RAM_WIDTH-1:0] bank0 [(2**RAM_ADDR_BITS)-1:0];
   reg [RAM_WIDTH-1:0] bank1 [(2**RAM_ADDR_BITS)-1:0];
   reg [RAM_WIDTH-1:0] bank1_out, bank0_out;   
   assign data2 = {bank1_out, 1'b0, bank0_out, 1'b0};
   assign bank0_we = we && addr1[10];
   assign bank1_we = we && !addr1[10];

   //  The forllowing code is only necessary if you wish to initialize the RAM 
   //  contents via an external file (use $readmemb for binary data)
   //initial
   //   $readmemh("<data_file_name>", <ram_name>, <begin_address>, <end_address>);

   always @(posedge clk) begin
      if (bank0_we) begin
         bank0[addr1[9:0]] <= data1;
      end
      if (bank1_we) begin
         bank1[addr1[9:0]] <= data1;
      end
      bank0_out <= bank0[addr2];
      bank1_out <= bank1[addr2];
   end
endmodule
