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
        for (let i = 0; i < document.points.length; i++) {

            let freq = document.points[i]["y"];
            ctx.fillStyle = noteColor(i / document.points.length)
            ctx.fillRect(document.points[i]["x"], freq_to_y_position(freq) - radius / 2, radius, radius);
            document.points[i].x -= speed;
        }

        // draw fft plot points
        ctx.beginPath();
        let move = 10;
        let zero_level = 200;
        let pos = (move, zero_level)
        let max = Math.max(document.fft)
        for (let i = 0; i < document.fft.length; i++) {
            // line
            ctx.moveTo(pos[0], pos[1]);
            let value = document.fft[i] / max * 100; //normalize to pixels
            pos = (pos[0] + move, zero_level - value)
            ctx.lineTo(pos[0], pos[1])

            // point
            ctx.fillStyle = "rgb(159,159,150)"
            ctx.fillRect(pos[0], pos[1], radius, radius);
        }
        ctx.stroke()


        requestAnimationFrame(animate);
    }
    define_globals()
    animate();
}

function OnParamChange(param, value) {
    if (param == 0) {

    }
}

function OnControlChange(ctrlTag, value) {
    if (ctrlTag == 0) {
        document.points.push({
            x: document.width,
            y: value
        })
    } else {
        document.fft[ctrlTag - 1] = value
    }
}