<?php

// กำหนดค่าการเชื่อมต่อฐานข้อมูล
$servername = "localhost";   // ชื่อเซิร์ฟเวอร์ 
$username = "root";          // ชื่อผู้ใช้ฐานข้อมูล 
$password = "";              // รหัสผ่าน 
$dbname = "db_sensor";       // ชื่อฐานข้อมูลที่ใช้เก็บข้อมูลเซนเซอร์

// สร้างการเชื่อมต่อกับฐานข้อมูล
$conn = new mysqli($servername,$username,$password,$dbname);

// ตรวจสอบการเชื่อมต่อ
if($conn->connect_error)
{
    die("Connection failed: " . $conn->connect_error); // ถ้าเชื่อมต่อไม่ได้ ให้หยุดและแสดงข้อความ error
}

// ตรวจสอบว่ามีค่าที่ส่งมาทาง POST ครบทุกตัวไหม
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
   isset($_POST['altitude']))
{
    // ถ้ามีครบ - เก็บค่าจาก POST ลงตัวแปร
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

    // รันคำสั่ง SQL และตรวจสอบผลลัพธ์
    if ($conn->query($sql) === TRUE) 
    {
        echo "Success"; // ถ้า insert สำเร็จ
    } 
    else 
    {
        echo "Error: " . $conn->error; // ถ้า insert ไม่สำเร็จ แสดง error
    }
}
else 
{
    // ถ้าไม่มีข้อมูล POST เข้ามาเลย
    echo "No data received";
}

// ปิดการเชื่อมต่อ (คอมเมนต์ไว้ แต่สามารถเปิดใช้งานได้)
// $conn->close();

?>
