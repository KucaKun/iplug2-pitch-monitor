function noteColor(goodness) {
    return `rgb(${Math.floor(255 - 255*goodness)}, ${Math.floor(255*goodness)}, 9)`
}

function freq_to_note_float(freq) {
    return 12 * Math.log(freq / 440) / Math.log(2)
}

function freq_to_y_position(freq) {
    let val = freq_to_note_float(freq) * (height / NOTE_COUNT);
    return height - (6 / 7 * height) - val
}

function testPoints(count) {
    let points = [];
    for (let i = 0; i < count; i++) {
        points.push({
            x: i,
            y: Object.values(NOTE_NAMES)[Math.floor(Math.random() * NOTE_COUNT)] //random freq
        })
    }
    return points;
}

function print(text) {
    document.infos.push(Math.floor(Date.now()) + " " + text)
    if (document.infos.length > 50) {
        document.infos.shift();
    }
}