<?php
$servername = "localhost";
$username   = "root";
$password   = "";
$dbname     = "test"; // เปลี่ยนเป็นชื่อฐานข้อมูลจริง

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// รับค่าจาก ESP32 (GET)
$lux          = $_GET['lux'];
$airtmp       = $_GET['airtmp'];
$airhum       = $_GET['airhum'];
$soilhum      = $_GET['soilhum'];
$soiltmp      = $_GET['soiltmp'];
$soilph       = $_GET['soilph'];
$soilec       = $_GET['soilec'];
$soiln        = $_GET['soiln'];
$soilp        = $_GET['soilp'];
$soilk        = $_GET['soilk'];
$lat          = $_GET['lat'];
$lon          = $_GET['lon'];
$au           = $_GET['au'];

// สร้างตาราง sensor_data ให้มีชื่อคอลัมน์ตรงกับด้านล่างนี้
$sql = "INSERT INTO sensor_data
        (lux, airtmp, airhum, soilhum, soiltmp, soilph, soilec, soiln, soilp, soilk, lat, lon, au)
        VALUES
        ($lux, $airtmp, $airhum, $soilhum, $soiltmp, $soilph, $soilec, $soiln, $soilp, $soilk, $lat, $lon, $au)";

if ($conn->query($sql) === TRUE) {
    echo "OK";
} else {
    echo "Error: " . $conn->error;
}

$conn->close();
?>

