module main;

reg a, b;

initial begin
        a = 1'b0;
	#5;
        while( a == 1'b0 )
          begin
           a = b;
           #5;
          end
end

initial begin
`ifndef VPI
        $dumpfile( "while1.1.vcd" );
        $dumpvars( 0, main );
`endif
	b = 1'b0;
        #20;
        $finish;
end

endmodule
