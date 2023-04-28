function main() {
    function animate() {
        ctx.clearRect(0, 0, width, height);

        // draw plot lines
        for (let i = 0; i < NOTE_COUNT; i++) {
            // horizontal lines
            ctx.beginPath()
            let y = freq_to_y_position(Object.values(NOTE_NAMES)[i])
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
            ctx.stroke();
            ctx.font = "13px Arial"
            ctx.fillStyle = "rgb(40,150,30)"
            ctx.fillText(Object.keys(NOTE_NAMES)[i], 0, y - 2);

            // draw info between lines
            ctx.font = "13px Arial"
            ctx.fillStyle = "rgb(190,0,30)"
            for (let category = 0; category < infos.length; category++) {
                if (infos[category].length) {
                    let last_info = infos[category][infos[category].length - 1]
                    ctx.fillText(
                        last_info["text"],
                        last_info["pos"], //x
                        y - 3 //y
                    );
                }
            }
        }

        // draw last freq points
        let radius = 4
        for (let i = 0; i < points.length; i++) {

            let freq = points[i]["y"];
            ctx.fillStyle = noteColor(i / points.length)
            ctx.fillRect(points[i]["x"], freq_to_y_position(freq) - radius / 2, radius, radius);
            points[i].x -= speed;
        }

        // draw plot points
        ctx.beginPath();
        let move = width / plot.length;
        let zero_level = height / 2;
        let maximum = Math.max(...plot)
        let minimum = Math.min(...plot)
        let range = maximum - minimum;
        let normalize = (x) => {
            return (x / range) * zero_level
        };
        let value = normalize(plot[0])
        let pos = [move, zero_level - value]
        for (let i = 1; i < plot.length; i++) {
            // line
            ctx.moveTo(pos[0], pos[1]);
            value = normalize(plot[i]);
            let y = zero_level - value
            let x = pos[0] + 1;
            x = Math.log10(fft_freqs[i]) * width / Math.log10(SAMPLE_RATE / 2)
            pos = [x, y]
            ctx.lineTo(pos[0], pos[1])

            // point
            ctx.fillStyle = "rgb(210,210,210)"
            ctx.fillRect(pos[0] - 1, pos[1] - 1, 2, 2);
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
        // print("freq: " + value, 0, 50);
        points.push({
            x: width,
            y: value
        })
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