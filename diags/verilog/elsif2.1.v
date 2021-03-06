`define FOO 1
`define ONE2ZERO 1

module main;

reg a, b;

initial begin
`ifdef FOO
`ifdef ZERO2ONE
	a = 1'b0;
	#1;
	a = 1'b1;
`elsif ONE2ZERO
	a = 1'b1;
	#1;
	a = 1'b0;
`else
	$display( "Hello world" );
`endif
`endif
	b = 1'b0;
	#1;
	b = 1'b1;
end

initial begin
`ifdef DUMP
        $dumpfile( "elsif2.1.vcd" );
        $dumpvars( 0, main );
`endif
        #10;
        $finish;
end

endmodule
