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