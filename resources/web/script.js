// FROM DELEGATE

function SPVFD(paramIdx, val) {
 console.log("paramIdx: " + paramIdx + " value:" + val);
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
function testPoints(count){
  let points = [];
  for(let i = 0; i < count; i++){
    points.push({x:i,y:(Math.sin(i)+1)*100})
  }
  return points;
}
function noteColor(goodness){
  return `rgb(${Math.floor(255 - 255*goodness)}, ${Math.floor(255*goodness)}, 9)`
}
function main(){
  // get canvas context and set initial variables
  const canvas = document.getElementById("canvas");
  const ctx = canvas.getContext("2d");
  const width = canvas.width;
  const height = canvas.height;
  const radius = 4;
  const points = testPoints(width);
  const speed = 1;
  function animate() {

    ctx.clearRect(0, 0, width, height);

    // draw plot lines
    for(let i = 0; i <= 10; i++){
      // plot line
      ctx.beginPath();
      let x = i * width/10;
      ctx.strokeStyle = "rgb(40,20,30)"
      ctx.moveTo(x, 0);
      ctx.lineTo(x, height);
      ctx.stroke();
    }

    // draw points
    for(let i = 0; i < points.length; i++){
      
      ctx.fillStyle = noteColor(i/points.length)
      ctx.fillRect(points[i]["x"], points[i]["y"],radius,radius);
      points[i].x -= speed;
      if(points[i].x < 0-radius){
        points[i].x+=width+radius;
      }
    }

    requestAnimationFrame(animate);
  }
  
  // start animation
  animate();
}