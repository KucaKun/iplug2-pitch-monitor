// FROM DELEGATE

function SPVFD(paramIdx, val) {
  console.log("paramIdx: " + paramIdx + " value:" + val);
  points.push({
    x: width,
    y: val
  })
  if (points.length > width) {
    points.shift();
  }
}

function SCVFD(ctrlTag, val) {
  console.log("SCVFD ctrlTag: " + ctrlTag + " value:" + val);
}

function SCMFD(ctrlTag, msgTag, msg) {
  //  var decodedData = window.atob(msg);
  console.log("SCMFD ctrlTag: " + ctrlTag + " msgTag:" + msgTag + "msg:" + msg);
}

function SAMFD(msgTag, dataSize, msg) {
  //  var decodedData = window.atob(msg);
  console.log("SAMFD msgTag:" + msgTag + " msg:" + msg);
}

function SMMFD(statusByte, dataByte1, dataByte2) {
  console.log("Got MIDI Message" + status + ":" + dataByte1 + ":" + dataByte2);
}

function SSMFD(offset, size, msg) {
  console.log("Got Sysex Message");
}

// FROM UI
// data should be a base64 encoded string
function SAMFUI(msgTag, ctrlTag = -1, data = 0) {
  var message = {
    "msg": "SAMFUI",
    "msgTag": msgTag,
    "ctrlTag": ctrlTag,
    "data": data
  };

  IPlugSendMsg(message);
}

function SMMFUI(statusByte, dataByte1, dataByte2) {
  var message = {
    "msg": "SMMFUI",
    "statusByte": statusByte,
    "dataByte1": dataByte1,
    "dataByte2": dataByte2
  };

  IPlugSendMsg(message);
}

// data should be a base64 encoded string
function SSMFUI(data = 0) {
  var message = {
    "msg": "SSMFUI",
    "data": data
  };

  IPlugSendMsg(message);
}

function EPCFUI(paramIdx) {
  var message = {
    "msg": "EPCFUI",
    "paramIdx": paramIdx,
  };

  IPlugSendMsg(message);
}

function BPCFUI(paramIdx) {
  var message = {
    "msg": "BPCFUI",
    "paramIdx": paramIdx,
  };

  IPlugSendMsg(message);
}

function SPVFUI(paramIdx, value) {
  var message = {
    "msg": "SPVFUI",
    "paramIdx": paramIdx,
    "value": value
  };

  IPlugSendMsg(message);
}


function main() {
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
  const NOTE_COUNT = Object.keys(NOTE_NAMES).length

  // get canvas context and set initial variables
  const canvas = document.getElementById("canvas");
  const ctx = canvas.getContext("2d");
  const width = canvas.width;
  const height = canvas.height;
  const radius = 4;
  const points = testPoints(width);
  const speed = 1;

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

  function animate() {

    ctx.clearRect(0, 0, width, height);

    // draw plot lines
    for (let i = 0; i < NOTE_COUNT; i++) {
      // vertical lines
      // ctx.beginPath();
      // let x = i * width/10;
      // ctx.strokeStyle = "rgb(40,20,30)"
      // ctx.moveTo(x, 0);
      // ctx.lineTo(x, height);
      // ctx.stroke();

      // horizontal lines
      ctx.beginPath()
      let y = freq_to_y_position(Object.values(NOTE_NAMES)[i])
      ctx.moveTo(0, y);
      ctx.lineTo(width, y);
      ctx.stroke();
      ctx.strokeStyle = "rgb(40,20,30)"
      ctx.fillText(Object.keys(NOTE_NAMES)[i], 0, y - 2);
    }

    // draw points
    for (let i = 0; i < points.length; i++) {

      let freq = points[i]["y"];
      ctx.fillStyle = noteColor(i / points.length)
      ctx.fillRect(points[i]["x"], freq_to_y_position(freq) - radius / 2, radius, radius);
      points[i].x -= speed;
    }

    requestAnimationFrame(animate);
  }

  // start animation
  animate();
}