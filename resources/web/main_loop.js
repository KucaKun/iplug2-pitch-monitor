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
                ctx.fillStyle = "rgba(242, 242, 242, 0.5)"
            } else {
                ctx.fillStyle = "rgba(200, 200, 200, 0.8)"
            }
            ctx.fillRect(0, y - key_width / 2, piano_width, key_width)

            // Lines between
            ctx.fillStyle = "rgba(1, 1, 1, 0.5)"
            ctx.fillRect(0, y + key_width / 2, piano_width, 2)

            // Note name
            ctx.font = "13px Arial"
            ctx.fillStyle = "rgb(40,150,30)"
            ctx.fillText(Object.keys(NOTE_NAMES)[i], piano_width, y + 5);
        }
        // draw info between lines
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

        // Draw frequency points
        let radius = 4
        for (let i = 0; i < hps_freq_points.length; i++) {
            let x = hps_freq_points[i]["x"] - radius / 2
            let freq = hps_freq_points[i]["y"];
            ctx.fillStyle = "green"
            let y = freq_to_linear(freq) - radius / 2
            ctx.fillRect(x, y, radius, radius);
            hps_freq_points[i].x -= speed;
        }
        for (let i = 0; i < fft_freq_points.length; i++) {
            let x = fft_freq_points[i]["x"] - radius / 2
            let freq = fft_freq_points[i]["y"];
            ctx.fillStyle = "blue"
            let y = freq_to_linear(freq) - radius / 2
            ctx.fillRect(x, y, radius, radius);
            fft_freq_points[i].x -= speed;
        }

        // Plot points
        ctx.beginPath();
        let move = piano_width / plot.length;
        let zero_level = height / 2;
        let maximum = 100
        let minimum = 0
        let range = maximum - minimum;
        let normalize = (t) => {
            return (t / range) * zero_level
        };
        let value = normalize(plot[0])
        let pos = [move, zero_level - value]
        for (let i = 1; i < plot.length; i++) {
            // line
            ctx.moveTo(pos[0], pos[1]);
            value = normalize(plot[i]);
            let y = zero_level - value
            let x = pos[0] + 1;
            x = Math.log10(fft_freqs[i]) * piano_width / Math.log10(SAMPLE_RATE / 2)
            pos = [x, y]
            ctx.lineTo(pos[0], pos[1])

            // point
            ctx.fillStyle = "rgb(210,210,210)"
            ctx.fillRect(pos[0] - 1, pos[1] - 1, 2, 2);

            ctx.fillStyle = "rgb(100,210,50)"
            let freq = Object.values(NOTE_NAMES)[i]
            ctx.fillRect(piano_width - normalize(plot[i]), freq_to_linear(freq), 2, 2);

        }
        ctx.strokeStyle = "rgb(210,210,210)"
        ctx.stroke()


        requestAnimationFrame(animate);
    }
    animate();
}

function OnParamChange(param, value) {
    if (param == 0) {

    }
}

function OnControlChange(ctrlTag, value) {

    if (ctrlTag == 0) {
        print("freq hps: " + value, 0, 50);
        hps_freq_points.push({
            x: piano_width,
            y: value
        })
        if (hps_freq_points.length == plot.length) {
            hps_freq_points.shift()
        }
    } else if (ctrlTag == 1) {
        print("freq fft: " + value, 0, 200);
        fft_freq_points.push({
            x: piano_width,
            y: value
        })
        if (fft_freq_points.length == plot.length) {
            fft_freq_points.shift()
        }


    } else {
        // print("plot1 x: " + ctrlTag + " y: " + value, 1, 200);
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