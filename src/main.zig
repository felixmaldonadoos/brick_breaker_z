const std = @import("std");

// ---------- Rendering ----------

const Style = struct {
    fill: []const u8 = "\x1b[47m \x1b[0m", // white background "square"
    empty: []const u8 = " ",
    x_scale: usize = 2, // widen cells so they look square in most terminals
    y_scale: usize = 1,
};

/// Generic grid renderer with a context + predicate.
/// Coordinates are Cartesian with (0,0) at bottom-left.
pub fn renderGridWith(
    out: anytype,
    width: usize,
    height: usize,
    ctx: anytype,
    isFilled: fn (@TypeOf(ctx), usize, usize) bool,
    style: Style,
) !void {
    var row: usize = 0;
    while (row < height) : (row += 1) {
        const y = height - 1 - row; // invert for Cartesian origin

        // Repeat each logical row y_scale times (for aspect)
        var rep_y: usize = 0;
        while (rep_y < style.y_scale) : (rep_y += 1) {
            var x: usize = 0;
            while (x < width) : (x += 1) {
                const cell = if (isFilled(ctx, x, y)) style.fill else style.empty;

                // Repeat each logical column x_scale times
                var rep_x: usize = 0;
                while (rep_x < style.x_scale) : (rep_x += 1) {
                    try out.print("{s}", .{cell});
                }
            }
            try out.print("\n", .{});
        }
    }
}

// ---------- Predicates / Contexts ----------

const FrameCtx = struct {
    canvas_w: usize,
    canvas_h: usize,

    /// True on the outer border (fence), false elsewhere.
    pub fn isFilled(self: FrameCtx, x: usize, y: usize) bool {
        const max_x = self.canvas_w - 1;
        const max_y = self.canvas_h - 1;
        return x == 0 or x == max_x or y == 0 or y == max_y;
    }
};

/// Build a frame context for an inner grid of size (inner_w, inner_h).
/// Fence lives on a canvas of (inner_w + 2) × (inner_h + 2).
fn getFrame(inner_w: usize, inner_h: usize) FrameCtx {
    return .{ .canvas_w = inner_w + 2, .canvas_h = inner_h + 2 };
}

const DotInner = struct { x: usize, y: usize };

/// Fence + inner-dot combined predicate (inner coords offset by +1 for the fence).
const CombinedInnerCtx = struct {
    frame: FrameCtx,
    dot: DotInner,

    pub fn isFilled(self: CombinedInnerCtx, x: usize, y: usize) bool {
        if (self.frame.isFilled(x, y)) return true;
        return x == self.dot.x + 1 and y == self.dot.y + 1;
    }
};

// ---------- Animation ----------

fn animateScan(
    out: anytype,
    inner_w: usize,
    inner_h: usize,
    dt_seconds: f64,
    style: Style,
) !void {
    const frame = getFrame(inner_w, inner_h);

    // Hide cursor, clear the screen, home the cursor.
    try out.print("\x1b[?25l\x1b[2J\x1b[H", .{});
    defer out.print("\x1b[?25h\n", .{}) catch {};

    const dt_ns: u64 = @intFromFloat(dt_seconds * @as(f64, std.time.ns_per_s));

    while (true) {
        // ROW-MAJOR: move right across each row, then go up to the next row.
        var y: usize = 0;
        while (y < inner_h) : (y += 1) {
            var x: usize = 0;
            while (x < inner_w) : (x += 1) {
                const ctx = CombinedInnerCtx{
                    .frame = frame,
                    .dot = .{ .x = x, .y = y },
                };

                // Draw from top-left each frame
                try out.print("\x1b[H", .{});
                try renderGridWith(out, frame.canvas_w, frame.canvas_h, ctx, CombinedInnerCtx.isFilled, style);

                std.time.sleep(dt_ns);
            }
        }
        // loops back to (0,0) automatically
    }
}

// ---------- Demo main ----------

pub fn main() void {
    const stdin = std.io.

}

pub fn main() !void {
    const out = std.io.getStdOut().writer();

    // Inner logical grid size (playfield inside the fence)
    const grid_w: usize = 10;
    const grid_h: usize = 10;

    // Seconds between frames — tweak freely
    const dt: f64 = 0.5;

    const style = Style{
        .fill = "\x1b[47m \x1b[0m",
        .empty = " ",
        .x_scale = 2,
        .y_scale = 1,
    };

    try animateScan(out, grid_w, grid_h, dt, style);
}
