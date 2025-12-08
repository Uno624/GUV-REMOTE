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
// ให้ใช้ชื่อ key แบบมีความหมายชัดเจนทั้งหมด
$lux          = $_GET['lux'] ?? NULL;
$airtmp       = $_GET['airtmp']?? NULL;
$airhum       = $_GET['airhum']?? NULL;
$soilhum      = $_GET['soilhum']?? NULL;
$soiltmp      = $_GET['soiltmp']?? NULL;
$soilph       = $_GET['soilph']?? NULL;
$soilec       = $_GET['soilec']?? NULL;
$soiln        = $_GET['soiln']?? NULL;
$soilp        = $_GET['soilp']?? NULL;
$soilk        = $_GET['soilk']?? NULL;
$lat          = $_GET['lat']?? NULL;
$lon          = $_GET['lon']?? NULL;
$au           = $_GET['au']?? NULL;

// **** อย่าลืมสร้างตาราง sensor_data ให้มีชื่อคอลัมน์ตรงกับด้านล่างนี้ ****
$sql = "INSERT INTO sensor_data 
        (lux, airtmp, airhum, soilhum, soiltmp, soilph, soilec, soiln, soilp, soilk, lat, lon, au)
        VALUES 
        ('$lux', '$airtmp', '$airhum', '$soilhum', '$soiltmp', '$soilph',
         '$soilec', '$soiln', '$soilp', '$soilk', '$lat', '$lon', '$au')";

$conn->query($sql);
?>
