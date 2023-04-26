function define_globals() {

    document.NOTE_NAMES = {
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
    document.NOTE_COUNT = Object.keys(NOTE_NAMES).length
    // get canvas context and set initial variables
    document.canvas = document.getElementById("canvas");
    document.ctx = canvas.getContext("2d");
    document.width = canvas.width
    document.height = canvas.height
    document.width = width;
    document.height = height;
    document.points = testPoints(width);
    document.infos = []
    document.fft = Array(1024)

    document.radius = 4;
    document.speed = 1 / 2;
}