// import "DPI-C" function void npc_assert();
module DpiAsserrt(
    input clock,
    input reset,
    input en
);

always@(posedge clk) begin
    if(!reset && en) begin
        // npc_assert();
    end
end

endmodule