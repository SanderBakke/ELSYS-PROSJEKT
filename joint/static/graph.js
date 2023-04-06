// Generate a random number between 0 and 100 to set the starting value of the progress bar
const startingValue = 0;
var current1 = startingValue;
var current2 = startingValue;
var current3 = startingValue;
var current4 = startingValue;
var current5 = startingValue;
function updateEnergi() {
  if (current1 >= 100 || current2 >= 100 || current3 >= 100 || current4 >= 100 || current5 >= 100){
    return;
  }
// Create a new XMLHttpRequest object
var xhttp = new XMLHttpRequest();

// Define the callback function
xhttp.onreadystatechange = function() {
  if (this.readyState == 4 && this.status == 200) {
    // Extract the energi value from the XML file
    var xmlDoc = this.responseXML;
    var energi1 = xmlDoc.getElementsByTagName("sektor1")[0].childNodes[0].nodeValue;
    var energi2 = xmlDoc.getElementsByTagName("sektor2")[0].childNodes[0].nodeValue;
    var energi3 = xmlDoc.getElementsByTagName("sektor3")[0].childNodes[0].nodeValue;
    var energi4 = xmlDoc.getElementsByTagName("sektor4")[0].childNodes[0].nodeValue;
    var energi5 = xmlDoc.getElementsByTagName("sektor5")[0].childNodes[0].nodeValue;

    // Check if any bars are at 100%
    if (current1<100){
      current1 = +current1 + +energi1;
    }
    if (current1 >100){
      current1 = 100;
    }
    if (current2<100){
      current2 = +current2 + +energi2;
    }
    if (current2 >100){
      current2 = 100;
    }
    if (current3<100){
      current3 = +current3 + +energi3;
    }
    if (current3 >100){
      current3 = 100;
    }
    if (current4<100){
      current4 = +current4 + +energi4;
    }
    if (current4 >100){
      current4 = 100;
    }
    if (current5<100){
      current5 = +current5 + +energi5;
    }
    if (current5 >100){
      current5 = 100;
    }
    
    // Update the energi display on the HTML page
    // Bar 1
    const progressBar1 = document.querySelector('#progress-bar1');
    progressBar1.style.height = `${current1}%`;
    document.querySelector('#progress-bar-text1').textContent = `${current1}%`;
    progressBar1.setAttribute('data-value', current1);
    console.log('Value added 1:',energi1);     
    console.log('Total value 1:', current1);
    // Bar 2
    const progressBar2 = document.querySelector('#progress-bar2');
    progressBar2.style.height = `${current2}%`;
    document.querySelector('#progress-bar-text2').textContent = `${current2}%`;
    console.log('Value added 2:',energi2);     
    console.log('Total value 2:', current2);
    // Bar 3
    const progressBar3 = document.querySelector('#progress-bar3');
    progressBar3.style.height = `${current3}%`;
    document.querySelector('#progress-bar-text3').textContent = `${current3}%`;
    console.log('Value added 3:',energi3);     
    console.log('Total value 3:', current3);
    // Bar 4
    const progressBar4 = document.querySelector('#progress-bar4');
    progressBar4.style.height = `${current4}%`;
    document.querySelector('#progress-bar-text4').textContent = `${current4}%`;
    console.log('Value added 4:',energi4);     
    console.log('Total value 4:', current4);
    // Bar 5
    const progressBar5 = document.querySelector('#progress-bar5');
    progressBar5.style.height = `${current5}%`;
    document.querySelector('#progress-bar-text5').textContent = `${current5}%`;
    console.log('Value added 5:',energi5);     
    console.log('Total value 5:', current5);
  }
};

// Generate a random query string to bust the cache
var queryString = "?t=" + new Date().getTime();

// Send the HTTP request to the server to fetch the XML file with cache-busting query string
xhttp.open("GET", "angle.xml", true);
xhttp.send();
}

function realisticLook(){
    var count = 200;
var defaults = {
origin: { y: 0.7 }
};

function fire(particleRatio, opts) {
confetti(Object.assign({}, defaults, opts, {
  particleCount: Math.floor(count * particleRatio)
}));
}

fire(0.25, {
spread: 26,
startVelocity: 55,
});
fire(0.2, {
spread: 60,
});
fire(0.35, {
spread: 100,
decay: 0.91,
scalar: 0.8
});
fire(0.1, {
spread: 120,
startVelocity: 25,
decay: 0.92,
scalar: 1.2
});
fire(0.1, {
spread: 120,
startVelocity: 45,
});
}

function random(){
function randomInRange(min, max) {
  return Math.random() * (max - min) + min;
}

confetti({
  angle: randomInRange(55, 125),
  spread: randomInRange(50, 70),
  particleCount: randomInRange(50, 100),
  origin: { y: 0.6 }
});
}
function fireworks(){
var number = 0;
let color = (0,0,0);
if (current1 == 100){
  number = "sektor 1";
  color= window.getComputedStyle(document.getElementById("progress-bar1")).getPropertyValue("background-color");
}
else if(current2 == 100){
  number = "sektor 2";
  color= window.getComputedStyle(document.getElementById("progress-bar2")).getPropertyValue("background-color");
}
else if(current3 == 100){
  number = "sektor 3";
  color= window.getComputedStyle(document.getElementById("progress-bar3")).getPropertyValue("background-color");
}
else if(current4 == 100){
  number = "sektor 4";
  color= window.getComputedStyle(document.getElementById("progress-bar4")).getPropertyValue("background-color");
}
else if(current5 == 100){
  number = "sektor 5";
  color= window.getComputedStyle(document.getElementById("progress-bar5")).getPropertyValue("background-color");
}

document.getElementById("text").style.backgroundColor = color;
document.getElementById("text").style.borderColor = "#000000" ;
document.getElementById('text').textContent = 'Congratulations ' + number + '!';
var duration = 15 * 1000;
var animationEnd = Date.now() + duration;
var defaults = { startVelocity: 30, spread: 360, ticks: 60, zIndex: 10 };

function randomInRange(min, max) {
  return Math.random() * (max - min) + min;
}

var interval = setInterval(function() {
  var timeLeft = animationEnd - Date.now();

  if (timeLeft <= 0) {
    return clearInterval(interval);
  }

  var particleCount = 200 * (timeLeft / duration);
  // since particles fall down, start a bit higher than random
  confetti(Object.assign({}, defaults, { particleCount, origin: { x: randomInRange(0.1, 0.3), y: Math.random() - 0.2 } }));
  confetti(Object.assign({}, defaults, { particleCount, origin: { x: randomInRange(0.7, 0.9), y: Math.random() - 0.2 } }));
}, 250);
}
let intervalId = setInterval(() => {
updateEnergi();

if (current1 == 100 || current2 == 100 || current3 == 100 || current4 == 100 || current5 == 100){
  clearInterval(intervalId);
  fireworks();
}
}, 1000);