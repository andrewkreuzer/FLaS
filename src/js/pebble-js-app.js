var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getWeather(latitude, longitude) {
	var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
		latitude + "&lon=" + longitude;
	xhrRequest(url, 'GET',
			function(responseText) {
				var json = JSON.parse(responseText);
				var temerature = MATH.round(json.main.temp - 273.15);
				var conditions = json.weather[0].main;
				console.log("It's " + conditions + "and " + temerature);
				Pebble.sendAppMessage({
					"WEATHER_TEMPERATUR_KEY":temperature + "\u00B0C",
					"WEATHER_CONDITIONS_KEY":conditions},
					function(e) { console.log("Weather info send succesfully")},
					function(e) { console.log("Error sending weather info")}
					);
	}
}
				


function locationSuccess(pos) {
	var coordinates = ps.coords;
	getWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.log("Error requesting location!");
	Pebble.sendAppMessage({
		"WEATHER_TEMPERATURE_KEY":"Unavailable",
		"WEATHER_CONDITIONS_KEY" :"N/A"
	});
}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getLocation();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getLocation();
  }                     
);

