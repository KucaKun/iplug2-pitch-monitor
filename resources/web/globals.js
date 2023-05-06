function define_globals() {
    SAMPLE_RATE = 96000
    BUFFER_SIZE = 8192
    NOTE_NAMES = {
        "C0": 16.35,
        "C#0/Db0": 17.32,
        "D0": 18.35,
        "D#0/Eb0": 19.45,
        "E0": 20.60,
        "F0": 21.83,
        "F#0/Gb0": 23.12,
        "G0": 24.50,
        "G#0/Ab0": 25.96,
        "A0": 27.50,
        "A#0/Bb0": 29.14,
        "B0": 30.87,
        "C1": 32.70,
        "C#1/Db1": 34.65,
        "D1": 36.71,
        "D#1/Eb1": 38.89,
        "E1": 41.20,
        "F1": 43.65,
        "F#1/Gb1": 46.25,
        "G1": 49.00,
        "G#1/Ab1": 51.91,
        "A1": 55.00,
        "A#1/Bb1": 58.27,
        "B1": 61.74,
        "C2": 65.41,
        "C#2/Db2": 69.30,
        "D2": 73.42,
        "D#2/Eb2": 77.78,
        "E2": 82.41,
        "F2": 87.31,
        "F#2/Gb2": 92.50,
        "G2": 98.00,
        "G#2/Ab2": 103.83,
        "A2": 110.00,
        "A#2/Bb2": 116.54,
        "B2": 123.47,
        "C3": 130.81,
        "C#3/Db3": 138.59,
        "D3": 146.83,
        "D#3/Eb3": 155.56,
        "E3": 164.81,
        "F3": 174.61,
        "F#3/Gb3": 185.00,
        "G3": 196.00,
        "G#3/Ab3": 207.65,
        "A3": 220.00,
        "A#3/Bb3": 233.08,
        "B3": 246.94,
        "C4": 261.63,
        "C#4/Db4": 277.18,
        "D4": 293.66,
        "D#4/Eb4": 311.13,
        "E4": 329.63,
        "F4": 349.23,
        "F#4/Gb4": 369.99,
        "G4": 392.00,
        "G#4/Ab4": 415.30,
        "A4": 440.00,
        "A#4/Bb4": 466.16,
        "B4": 493.88,
        "C5": 523.25,
        "C#5/Db5": 554.37,
        "D5": 587.33,
        "D#5/Eb5": 622.25,
        "E5": 659.25,
        "F5": 698.46,
        "F#5/Gb5": 739.99,
        "G5": 783.99,
        "G#5/Ab5": 830.61,
        "A5": 880.00,
        "A#5/Bb5": 932.33,
        "B5": 987.77,
        "C6": 1046.50,
        "C#6/Db6": 1108.73,
        "D6": 1174.66,
        "D#6/Eb6": 1244.51,
        "E6": 1318.51,
        "F6": 1396.91,
        "F#6/Gb6": 1479.98,
        "G6": 1567.98,
        "G#6/Ab6": 1661.22,
        "A6": 1760.00,
        "A#6/Bb6": 1864.66,
        "B6": 1975.53
    }
    NOTE_COUNT = Object.keys(NOTE_NAMES).length
    canvas = document.getElementById("canvas");
    ctx = canvas.getContext("2d");
    width = canvas.width
    piano_width = width * 0.8;
    height = canvas.height
    main_freq_points = [];
    side_freq_points = [];
    infos = Array(1024)
    for (let category = 0; category < infos.length; category++) {
        infos[category] = []
    }
    plot = Array(512)
    plots = {
        "main": {
            size: 512,
            points: []
        }
    }
    fft_input_size = 512
    fft_freqs = []
    for (i = 0; i < fft_input_size; i++) {
        fft_freqs.push(i * SAMPLE_RATE / BUFFER_SIZE)
    }

    is_dragging = false;
    y_drag_zero = 0
    y_drag_offset = 500
    key_width = 18

    speed = 1 / 2;

    // HUD 
    FFT_FREQS_TEXTS = false
    NOTE_FREQS_TEXTS = false
    PRINT_INFOS = false
    main()
    register_events()
}
window.addEventListener("load", define_globals)