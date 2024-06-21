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




setInterval(function getData()
{
    var xhttp.onreadystatechange = function()
    
    {
        if(this.readyState == 4 && this.status == 200)
        {
            document.getElementsbyId("valeurtemperatureIN").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "tempIN", true);
    whttp.send();
}, 6000);