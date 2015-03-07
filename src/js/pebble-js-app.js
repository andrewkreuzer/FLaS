var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.respnseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude;

  xhrRequest(url, 'GET',
    function(responseText) {
      var json = JSON.parse(requestText);

      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperatuer is " + temperature);

      var conditions = json.weather[0].main;
      console.log("Conditions are " + conditions);

      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent");
	},
	function(e) {
          console.log("Error weather not sent");
	}
      );
    }
  );
}

function locationError(err) {
  console.log("Error requesting location");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout:15000, maximumAge: 60000}
  );
}

Pebble.addEventListener( 'ready', 
  function(e) {
    console.log("PebbleKit ready!");
    getWeather();
  }
);


Pebble.addEventListener('appmessage',
  fuction(e) {
    console.log("App Message");
    getWeather();
  }
);

