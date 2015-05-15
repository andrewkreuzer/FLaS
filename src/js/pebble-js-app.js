var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
		  callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function weatherIcontoInt(weatherIcon) {
		switch(weatherIcon) {
			case "01d":
				result = 0;
				break;
			case "01n":
				result = 1;
				break;
			case "02d":
				result = 6;
				break;
			case "02n":
				result = 7;
				break;
			case "03d":
			case "04d":
			case "03n":
			case "04n":
				result = 5;
				break;
			case "09d":
			case "09n":
				result = 2;
				break;
			case "10d":
			case"10n":
				result = 9;
				break;
			case "11d":
			case "11n":
				result = 8;
				break;
			case "13d":
			case "13n":
				result = 3;
				break;
			case "50d":
			case "50n":
				result = 4;
			default:
				result = 10;
		}

		return result;
}

function getWeather(latitude, longitude) {
	var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
		latitude + "&lon=" + longitude;
	xhrRequest(url, 'GET', 
		function(responseText) {
			var json = JSON.parse(responseText);
			var temperature = Math.round(json.main.temp - 273.15).toString();
			console.log(typeof temperature);
			var conditions = json.weather[0].main;
			var weatherIcon = json.weather[0].icon;
			var iconNum = weatherIcontoInt(weatherIcon);

			Pebble.sendAppMessage({
					"WEATHER_TEMPERATURE_KEY": temperature + "\u00B0",
					"WEATHER_CONDITIONS_KEY": conditions,
					"WEATHER_ICON_KEY": iconNum
					},
					function(e) { console.log("Weather info sent succesfully | Temperature: " + temperature + ", Conditions: " + conditions + ", Icon Number: " + iconNum);},
					function(e) { console.log("Error sending weather info "
							+ temperature + conditions + iconNum);}
			);

		}
	);
}


function locationSuccess(pos) {
	var coordinates = pos.coords;
	getWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.log("Error requesting location!");
	Pebble.sendAppMessage({
		"WEATHER_TEMPERATURE_KEY":"Unavailable",
		"WEATHER_CONDITIONS_KEY" :"N/A",
		"WEATHER_ICON_KEY"       : 10
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
	console.log(appmessage)
    getLocation();
  }                     
);

