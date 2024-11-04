module top (
    input   wire        clk,
    input   wire        rst_n,

    input   wire [1:0]  a,
    input   wire [1:0]  b,

    output  reg	 [3:0]  c
);
reg     [3:0] d;
wire    [3:0] e;

always @(posedge clk) begin
    if (rst_n == 0)
        c <= 0;
    else
        c <= a + b;
end

always @(posedge clk) begin
    d <= c;
end

assign e = a + b;

endmodule
