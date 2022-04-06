function _GET(identifier) {
    var result = undefined, tmp = [];

    var items = window.location.search.substr(1).split("&");

    for (var index = 0; index < items.length; index++) {
        tmp = items[index].split("=");

        if (tmp[0] === identifier){
            result = decodeURIComponent(tmp[1]);
        }
    }

    return result;
}


	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
        		var myObj = JSON.parse(this.responseText);
			if (_GET("s1t")==null){document.getElementById("s1t").value = myObj.Servo1.trim;}else{document.getElementById("s1t").value = _GET("s1t");}

			if (_GET("s1x")==null){document.getElementById("s1x").value = myObj.Servo1.max;}else{document.getElementById("s1x").value = _GET("s1x");}

			if (_GET("s1n")==null){document.getElementById("s1n").value = myObj.Servo1.min;}else{document.getElementById("s1n").value = _GET("s1n");}
	
			if (_GET("s1f")==null){document.getElementById("s1f").value = myObj.Servo1.failsave;}else{document.getElementById("s1f").value = _GET("s1f");}

			if (_GET("s1e")==null){document.getElementById("s1e").value = myObj.Servo1.expo;}else{document.getElementById("s1e").value = _GET("s1e");}

			
		if (_GET("s1r")==null){document.getElementById("s1r").checked = myObj.Servo1.reverse;}else{document.getElementById("s1r").checked = JSON.parse(_GET("s1r"));}

			
			if (_GET("Ftime")==null){document.getElementById("Ftime").value = myObj.Ftime;}else{document.getElementById("Ftime").value = _GET("Ftime");}

			if (_GET("s1s")==null){document.getElementById("s1s").value = myObj.Servo1.rate;}else{document.getElementById("s1s").value = _GET("s1s");}
	
			if (_GET("Mode")==null){document.getElementById("Mode").value = myObj.Mode;}else{document.getElementById("Mode").value = _GET("Mode");}

			if (_GET("rcname")==null){
				document.getElementById("rcname").value = myObj.RCname;
				document.getElementById("rc").innerHTML = myObj.RCname
			}else{
				document.getElementById("rcname").value = _GET("rcname");
				document.getElementById("rc").innerHTML = _GET("rcname");
			}


			if (_GET("RCSSID")==null){document.getElementById("RCSSID").value = myObj.SSID;}else{document.getElementById("RCSSID").value = _GET("RCSSID");}


			if (_GET("RCPASS")==null){document.getElementById("RCPASS").value = myObj.Pass;}else{document.getElementById("RCPASS").value = _GET("RCPASS");}
			
		if (_GET("WiFiAP")==null){document.getElementById("WiFiAP").checked = myObj.WiFiAP;}else{document.getElementById("WiFiAP").checked = JSON.parse(_GET("WiFiAP"));}
			

    }
};
xmlhttp.open("GET", "config.json", true);
xmlhttp.send();
