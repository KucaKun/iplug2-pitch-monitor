function main() {
    print("green: hps", 3, 500)
    print("blue: fft", 4, 600)

    function animate() {
        ctx.setTransform(1, 0, 0, 1, 0, 0);
        ctx.clearRect(0, 0, width, 10000);
        ctx.translate(0, -y_drag_offset)
        for (let i = 0; i < NOTE_COUNT; i++) {
            // Piano keys
            ctx.beginPath()
            let note_val = Object.values(NOTE_NAMES)[i]
            let note_name = Object.keys(NOTE_NAMES)[i]
            let y = freq_to_linear(note_val)
            if (note_name.length == 2) {
                ctx.fillStyle = WHITE_KEY_COLOR
            } else {
                ctx.fillStyle = BLACK_KEY_COLOR
            }
            ctx.fillRect(0, y - key_width / 2, piano_width, key_width)

            // Lines between
            ctx.fillStyle = "rgba(1, 1, 1, 0.5)"
            ctx.fillRect(0, y + key_width / 2, piano_width, 2)

            // Note name
            ctx.font = "13px Arial"
            ctx.fillStyle = "rgb(40,150,30)"
            ctx.fillText(Object.keys(NOTE_NAMES)[i], piano_width, y + 5);

            // Note freq
            if (NOTE_FREQS_TEXTS) {
                ctx.fillText(Object.values(NOTE_NAMES)[i], piano_width + 60, y + 5);
            }
        }
        // draw info between lines
        if (PRINT_INFOS) {
            ctx.font = "13px Arial"
            ctx.fillStyle = "rgb(190,0,30)"
            for (let category = 0; category < infos.length; category++) {
                let count = infos[category].length;
                for (let i = 1; i <= count; i++) {
                    let y = freq_to_linear(Object.values(NOTE_NAMES)[i])
                    let info = infos[category][count - i] // go from the newest
                    ctx.fillText(info["text"], info["pos"], y + 5);
                }
            }
        }

        // Draw frequency points
        let radius = 4
        // main input
        function draw_freq_points(freq_points, color) {
            let last_pos = []
            for (let i = 0; i < freq_points.length; i++) {
                let x = freq_points[i]["x"]
                let freq = freq_points[i]["y"];
                let y = freq_to_linear(freq)

                ctx.fillStyle = color;
                fillCircle(x, y, radius / 2, ctx)
                if (last_pos && Math.abs(last_pos[1] - y) < 100) {
                    ctx.beginPath()
                    ctx.moveTo(last_pos[0], last_pos[1])

                    ctx.strokeStyle = color
                    ctx.quadraticCurveTo(x, y, x, y)
                    ctx.stroke()
                }
                last_pos = [x, y]
                freq_points[i].x -= speed;
            }
        }
        draw_freq_points(main_freq_points, "rgba(49,20,240,1)")
        draw_freq_points(side_freq_points, "rgba(49,240,12,1)")

        // Plot ffts
        let zero_level = height / 2;
        let maximum = 100
        let minimum = 0
        let range = maximum - minimum;
        let normalize = (t) => {
            return (t / range) * zero_level
        };
        for (let x = 0; x < plot.length; x++) {
            // line
            let value = normalize(plot[x]);
            let y = zero_level - value
            let log_x = (Math.log10(index_to_freq(x)) / Math.log10(SAMPLE_RATE / 2));

            // side plot
            let radius = 6;
            ctx.fillStyle = "rgba(80,85,250,0.5)"
            ctx.fillRect(piano_width - normalize(plot[x]) - radius / 2, freq_to_linear(index_to_freq(x)) - radius / 2, radius, radius);
            ctx.fillRect(piano_width - normalize(plot[x]) - radius / 2, freq_to_linear(index_to_freq(x)) - radius / 4, normalize(plot[x]), radius / 2);

            if (FFT_FREQS_TEXTS) {
                ctx.font = "13px Arial"
                ctx.fillStyle = "rgb(40,100,130)"
                ctx.fillText(index_to_freq(x), piano_width - 60, freq_to_linear(index_to_freq(x)) + 5);
            }
        }


        requestAnimationFrame(animate);
    }
    animate();
}

async function OnDownloadSubmit(e) {
    let url = document.getElementById("yt-url").value
    if (url.length) {
        let utf8Encode = new TextEncoder();
        let uint8 = utf8Encode.encode(url + "\0"); // zero ended
        var bin = String.fromCharCode.apply(null, uint8);
        SAMFUI(0, 0, window.btoa(bin))
        document.getElementById("yt-url").value = ""
    }
}

function OnParamChange(param, value) {
    if (param == 0) {

    }
}

function OnControlChange(ctrlTag, value) {

    if (ctrlTag == 0 && value > 0) {
        print("freq hps: " + value, 0, 50);
        main_freq_points.push({
            x: piano_width,
            y: value
        })
        if (main_freq_points.length == plot.length) {
            main_freq_points.shift()
        }
    } else if (ctrlTag == 1 && value > 0) {
        print("freq fft: " + value, 0, 200);
        side_freq_points.push({
            x: piano_width,
            y: value
        })
        if (side_freq_points.length == plot.length) {
            side_freq_points.shift()
        }
    } else {
        plot[ctrlTag - 1024] = value
    }
}

function OnMessage(msgTag, decodedData) {
    if (msgTag == 0) {
        console.log(decodedData)
        print(decodedData, 2, 400)
    }
}

function OnMouseUp(e) {
    is_dragging = false;
}

function OnMouseDown(e) {
    var y = e.clientY;
    is_dragging = true;
    y_drag_zero = (y + y_drag_offset);
}

function OnMouseMove(e) {
    if (is_dragging) {
        var y = e.clientY;
        y_drag_offset = y_drag_zero - y;
    }
}

function OnScroll(e) {
    let delta = Math.sign(e.deltaY) * 2;
    let note_delta = freq_to_linear(Object.values(NOTE_NAMES)[1]) - freq_to_linear(Object.values(NOTE_NAMES)[0])
    y_drag_offset -= delta * note_delta;
}

function OnKeyDown(e) {}