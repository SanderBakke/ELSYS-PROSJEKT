// Generate a random number between 0 and 100 to set the starting value of the progress bar
const startingValue = 0;
var current = startingValue;

function updateAngle() {
// Create a new XMLHttpRequest object
var xhttp = new XMLHttpRequest();

// Define the callback function
xhttp.onreadystatechange = function() {
  if (this.readyState == 4 && this.status == 200) {
    // Extract the energi value from the XML file
    var xmlDoc = this.responseXML;
    current = xmlDoc.getElementsByTagName("angle")[0].childNodes[0].nodeValue;
    

    screenPercentage = current/180*100;
    
    // Update the circles position
    const dot = document.getElementById('circle');
    dot.style.left = `${screenPercentage}%`;
    console.log('Angle:', current);

   document.getElementById('text').textContent = 'Angle: ' + `${current}`;
  }
};

// Generate a random query string to bust the cache
var queryString = "?t=" + new Date().getTime();

// Send the HTTP request to the server to fetch the XML file with cache-busting query string
xhttp.open("GET", "angle.xml" + queryString, true);
xhttp.send();
}


setInterval(updateAngle, 10);