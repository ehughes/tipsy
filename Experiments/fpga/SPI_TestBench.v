`timescale 1ns / 1ns

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   14:49:17 09/12/2013
// Design Name:   PWM
// Module Name:   C:/WORK/FPGA/LAB3_5/PWM_TestBench.v
// Project Name:  LAB3_5
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: PWM
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module SPI_TestBench;

	// Inputs
	reg clk, rst, done;
	reg [7:0] spi_out;
	wire [7:0] dout;

	// Instantiate the Unit Under Test (UUT)
	  spi_fsm spi_handler (
	    .clk(clk),
	    .rst(rst),
	    .data_in(spi_out),
	    .data_ready_in(done),
	    .data_out(dout)
	  );

always begin
	 #100  clk = ~clk;
end
	
initial begin

 	$display ("time\t clk rst spi_out done dout");
 	$monitor ("%g\t %b %b %b %b %b", $time, clk, rst, spi_out, done, dout);
	// Initialize Inputs
	clk = 0;
	rst = 0;
	spi_out = 0;
	done = 0;

	#5000;
	rst = 1;
	#5000;
	rst = 0;

	// Wait 100 ns for global reset to finish
	#5000;
	spi_out = 8'h77;
	done = 1;
	#5000;
	done = 0;
	#5000;
	spi_out = 8'h01;
	done = 1;
	#5000;
	done = 0;
	#5000;
	spi_out = 8'h23;
	done = 1;
	#5000;
	done = 0;
	#5000;
	spi_out = 8'h00;
	done = 1;
	#5000;
	done = 0;
	#5000;
	spi_out = 8'h01;
	done = 1;
	#5000;
	done = 0;
	#5000;
	spi_out = 8'haa;
	done = 1;
	#5000;
	done = 0;
	#5000;
	spi_out = 8'haa;
	done = 1;
	#5000;
	done = 0;

	// Add stimulus here
	#5000 $finish;
end
      
endmodule

