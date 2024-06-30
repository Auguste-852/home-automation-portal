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

$(document).ready(function(){
    $("#appliquer").click(function(){
        event.preventDefault(); // Empêcher le comportement par défaut du bouton (soumission de formulaire)
        
        var valeur = $("#AmountFood").val();
        $.post("AmountFood",{
            valueAmountFood: value
        });
    });
});

$(document).ready(function(){
    $("#apply2").click(function(event){
        
        event.preventDefault(); // Empêcher le comportement par défaut du bouton (soumission de formulaire)

        // Récupérer les valeurs des champs du formulaire
        var doorInterval = $("#doorInterval").val();
        var feedInterval = $("#feedInterval").val();
        var foodQuantity = $("#foodQuantity").val();

        // Envoyer les valeurs au serveur via une requête POST
        $.post("updateSettings", {
            doorInterval: doorInterval,
            feedInterval: feedInterval,
            foodQuantity: foodQuantity
        });
    });
});

function getData()
{
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementById("doorInterval").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "ReaddoorInterval", true);
    xhttp.send();
}

window.onload = function() {
    getData();
};