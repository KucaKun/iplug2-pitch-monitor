function define_globals() {
    canvas = document.getElementById("canvas");
    ctx = canvas.getContext("2d");
    width = canvas.width
    height = canvas.height
    NOTE_NAMES = {
        "C2": 65.41,
        "C#2/Db2 ": 69.30,
        "D2": 73.42,
        "D#2/Eb2 ": 77.78,
        "E2": 82.41,
        "F2": 87.31,
        "F#2/Gb2 ": 92.50,
        "G2": 98.00,
        "G#2/Ab2 ": 103.83,
        "A2": 110.00,
        "A#2/Bb2 ": 116.54,
        "B2": 123.47,
        "C3": 130.81,
        "C#3/Db3 ": 138.59,
        "D3": 146.83,
        "D#3/Eb3 ": 155.56,
        "E3": 164.81,
        "F3": 174.61,
        "F#3/Gb3 ": 185.00,
        "G3": 196.00,
        "G#3/Ab3 ": 207.65,
        "A3": 220.00,
        "A#3/Bb3 ": 233.08,
        "B3": 246.94,
        "C4": 261.63,
        "C#4/Db4 ": 277.18,
        "D4": 293.66,
        "D#4/Eb4 ": 311.13,
        "E4": 329.63,
        "F4": 349.23,
        "F#4/Gb4 ": 369.99,
        "G4": 392.00,
        "G#4/Ab4 ": 415.30,
        "A4": 440.00,
        "A#4/Bb4 ": 466.16,
        "B4": 493.88,
        "C5": 523.25,
        "C#5/Db5 ": 554.37,
        "D5": 587.33,
        "D#5/Eb5 ": 622.25
    }
    NOTE_COUNT = Object.keys(NOTE_NAMES).length
    SAMPLE_RATE = 96000
    BUFFER_SIZE = 2048
    // get canvas context and set initial variables
    points = testPoints(width);
    infos = Array(1024)
    for (let category = 0; category < infos.length; category++) {
        infos[category] = []
    }
    plot = Array(512)

    fft_input_size = 512
    fft_freqs = []
    for (i = 0; i < fft_input_size; i++) {
        fft_freqs.push(i * SAMPLE_RATE / BUFFER_SIZE)
    }

    speed = 1 / 2;
    main()
}
window.addEventListener("load", define_globals)