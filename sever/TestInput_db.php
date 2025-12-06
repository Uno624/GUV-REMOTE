<?php

$servername = "localhost";
$username = "admin";
$password = "112233";
$dbname = "db_sensor";


$conn = new mysqli($servername,$username,$password,$dbname);

if($conn->connect_error)
{
 die("Connection failed: " . $conn->connect_error);
}
if(isset($_POST['lux']) &&
   isset($_POST['temperature']) &&
   isset($_POST['humidity']) &&
   isset($_POST['soil_temp']) &&
   isset($_POST['soil_hum']) &&
   isset($_POST['ph']) &&
   isset($_POST['soil_ec']) &&
   isset($_POST['nitrogen']) &&
   isset($_POST['phosphorus']) &&
   isset($_POST['potassium'])&&
   isset($_POST['latitude'])&&
   isset($_POST['longitude'])&&
   isset($_POST['altitude']))
   {
      $Lux         =	$_POST['lux'];
      $Temperature =	$_POST['temperature'];
      $Humidity    =	$_POST['humidity'];
      $Soil_Temp   =	$_POST['soil_temp']; 
      $Soil_Hum    =	$_POST['soil_hum'];
      $PH	   =	$_POST['ph'];
      $Soil_EC     =	$_POST['soil_ec'];
      $Nitrogen    =	$_POST['nitrogen'];
      $Phosphorus  =	$_POST['phosphorus'];
      $Potassium   =	$_POST['potassium'];
      $Latitude    =	$_POST['latitude'];
      $Longitude   =	$_POST['longitude'];
      $Altitude    =	$_POST['altitude'];

      $sql = "INSERT INTO sensor_data
	       (Lux, Temperature, Humidity, Soil_Temp, Soil_Hum, PH, Soil_EC, Nitrogen, Phosphorus, Potassium, Latitude, Longitude, Altitude)
	       VALUES
	       ('$Lux', '$Temperature', '$Humidity', '$Soil_Temp', '$Soil_Hum', '$PH', '$Soil_EC', '$Nitrogen', '$Phosphorus', '$Potassium', '$Latitude', '$Longitude', '$Altitude')";	


      if ($conn->query($sql) === TRUE) 
      {
	 echo "Success";
      } 
      else 
      {
	 echo "Error: " . $conn->error;
      }
   }
else 
{
   echo "No data received";
}


$conn->close();
?>