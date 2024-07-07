document.addEventListener("DOMContentLoaded", function() {
    function updateClock() {
        fetch('/time')
            .then(response => response.text())
            .then(timeString => {
                document.getElementById("clock").textContent = timeString;
            })
            .catch(error => console.error('Error fetching time:', error));
    }

    updateClock(); // Initial call to display clock immediately
    setInterval(updateClock, 60000); // Update the clock every minute
});

function DoorClose() 
{
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "close", true); 
    xhttp.send();
}

function DoorOpen()
{
    var xhttp = new XMLHttpRequest(); 
    xhttp.open("GET", "open", true);
    xhttp.send();
}

$(document).ready(function() {
    $("#apply2").click(function(event) {
        event.preventDefault(); // Empêcher le comportement par défaut du bouton (soumission de formulaire)

        // Récupérer les valeurs des champs du formulaire
        var doorInterval = $("#doorInterval").val();
        var feedInterval = $("#feedInterval").val();
        var foodQuantity = $("#foodQuantity").val();
        var photoresistorThreshold = $("#photoresistorThreshold").val();

        // Envoyer les valeurs au serveur via une requête POST
        $.post("updateSettings", {
            doorInterval: doorInterval,
            feedInterval: feedInterval,
            foodQuantity: foodQuantity,
            photoresistorThreshold: photoresistorThreshold
        }).done(function() {
            $("#input_error2").text("Settings updated successfully!").css("color", "green");
        }).fail(function() {
            $("#input_error2").text("Error updating settings!").css("color", "red");
        });
    });
});

function updateThresholdValue(value) {
    $("#thresholdValue").text(value);
}

function getData()
{
  /*  
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("DoorState").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "/ReadDoorState", true);
    xhttp.send();



    var xhttp2 = new XMLHttpRequest();

    xhttp2.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
            {
                document.getElementById("timeSinceLastEat").innerHTML = this.responseText;
            }
            };

    xhttp2.open("GET", "ReadTimeSinceLastEat", true);
    xhttp2.send();
*/
    var xhttp3 = new XMLHttpRequest();

    xhttp3.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
            {
                document.getElementById("DoorInterval").innerHTML = this.responseText;
            }
            };

    xhttp3.open("GET", "ReadDoorInterval", true);
    xhttp3.send();


/*
    var xhttp4 = new XMLHttpRequest();

    xhttp4.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
            {
                document.getElementById("FeedInterval").innerHTML = this.responseText;
            }
            };

    xhttp4.open("GET", "ReadFeedInterval", true);
    xhttp4.send();

    
    

    var xhttp5 = new XMLHttpRequest();

    xhttp5.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
            {
                document.getElementById("FoodQuantity").innerHTML = this.responseText;
            }
            };

    xhttp5.open("GET", "ReadFoodQuantity", true);
    xhttp5.send();

    
    
    
    var xhttp6 = new XMLHttpRequest();

    xhttp6.onreadystatechange = function() 
    {
        if (this.readyState == 4 && this.status == 200) 
            {
                document.getElementById("PhotoresistorThreshold").innerHTML = this.responseText;
            }
            };

    xhttp6.open("GET", "ReadPhotoresistorThreshold", true);
    xhttp6.send();
*/
    }

window.onload = function() {
    getData();
};