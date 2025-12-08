<?php
include 'db.php'; // เชื่อมต่อฐานข้อมูล โดยใช้การตั้งค่าจาก db.php

// ตรวจสอบว่ามีข้อมูล POST ครบทุกตัวหรือไม่
if(isset($_POST['lux']) &&
   isset($_POST['temperature']) &&
   isset($_POST['humidity']) &&
   isset($_POST['soil_temp']) &&
   isset($_POST['soil_hum']) &&
   isset($_POST['ph']) &&
   isset($_POST['soil_ec']) &&
   isset($_POST['nitrogen']) &&
   isset($_POST['phosphorus']) &&
   isset($_POST['potassium']) &&
   isset($_POST['latitude']) &&
   isset($_POST['longitude']) &&
   isset($_POST['altitude'])) {

    // เก็บค่าจาก POST ลงตัวแปร
    $Lux         = $_POST['lux'];
    $Temperature = $_POST['temperature'];
    $Humidity    = $_POST['humidity'];
    $Soil_Temp   = $_POST['soil_temp'];
    $Soil_Hum    = $_POST['soil_hum'];
    $PH          = $_POST['ph'];
    $Soil_EC     = $_POST['soil_ec'];
    $Nitrogen    = $_POST['nitrogen'];
    $Phosphorus  = $_POST['phosphorus'];
    $Potassium   = $_POST['potassium'];
    $Latitude    = $_POST['latitude'];
    $Longitude   = $_POST['longitude'];
    $Altitude    = $_POST['altitude'];

    // สร้างคำสั่ง SQL สำหรับเพิ่มข้อมูลลงในตาราง sensor_data
    $sql = "INSERT INTO sensor_data 
            (Lux, Temperature, Humidity, Soil_Temp, Soil_Hum, PH, Soil_EC, Nitrogen, Phosphorus, Potassium, Latitude, Longitude, Altitude) 
            VALUES 
            ('$Lux', '$Temperature', '$Humidity', '$Soil_Temp', '$Soil_Hum', '$PH', '$Soil_EC', '$Nitrogen', '$Phosphorus', '$Potassium', '$Latitude', '$Longitude', '$Altitude')";

    // รัน SQL และตรวจสอบผลลัพธ์
    if ($conn->query($sql) === TRUE) {
        echo "Success"; // ถ้า insert สำเร็จ
    } else {
        echo "Error: " . $conn->error; // ถ้า insert ไม่สำเร็จ
    }
} else {
    echo "No data received"; // ถ้าไม่มีข้อมูล POST เข้ามาเลย
}

// ปิดการเชื่อมต่อฐานข้อมูล
$conn->close();
?>