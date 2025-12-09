<?php
include 'db.php'; // เชื่อมต่อฐานข้อมูล โดยเรียกใช้ไฟล์ db.php

// ดึงข้อมูลจากตาราง sensor_data โดยเลือกคอลัมน์ที่ต้องการ และเรียงลำดับจากใหม่สุด (DESC)
// จำกัดจำนวนแถวที่ดึงมาแค่ 10 แถวล่าสุด
$sql = "SELECT id, lux, airtmp, airhum, soilhum, soiltmp, soilph, soilec, soiln, soilp, soilk, lat, lon, au, created_at
        FROM sensor_data 
        ORDER BY created_at DESC 
        LIMIT 500";;

// รันคำสั่ง SQL และเก็บผลลัพธ์ไว้ในตัวแปร $result
$result = $conn->query($sql); // ต้องอยู่ก่อนปิด connection

// เริ่มต้นการแสดงตาราง HTML
echo "<table border='1'>";

// สร้างหัวตาราง (table header)
echo "<tr>
        <th>ID</th>
        <th>Lux</th>
        <th>Temperature</th>
        <th>Humidity</th>
        <th>Soil Temp</th>
        <th>Soil Hum</th>
        <th>PH</th>
        <th>Soil EC</th>
        <th>Nitrogen</th>
        <th>Phosphorus</th>
        <th>Potassium</th>
        <th>Latitude</th>
        <th>Longitude</th>
        <th>Altitude</th>
        <th>Time</th>
      </tr>";

// ตรวจสอบว่ามีข้อมูลในผลลัพธ์หรือไม่
if ($result->num_rows > 0) {
    // ถ้ามีข้อมูล → วนลูปแสดงแต่ละแถว
    while($row = $result->fetch_assoc()) {
        echo "<tr>";
        echo "<td>".$row['id']."</td>";
        echo "<td>".$row['lux']."</td>";
        echo "<td>".$row['airtmp']."</td>";
        echo "<td>".$row['airhum']."</td>";
        echo "<td>".$row['soilhum']."</td>";
        echo "<td>".$row['soiltmp']."</td>";
        echo "<td>".$row['soilph']."</td>";
        echo "<td>".$row['soilec']."</td>";
        echo "<td>".$row['soiln']."</td>";
        echo "<td>".$row['soilp']."</td>";
        echo "<td>".$row['soilk']."</td>";
        echo "<td>".$row['lat']."</td>";
        echo "<td>".$row['lon']."</td>";
        echo "<td>".$row['au']."</td>";
        echo "<td>".$row['created_at']."</td>";
        echo "</tr>";
    }
} else {
    // ถ้าไม่มีข้อมูล → แสดงข้อความ "No data found" ในตาราง
    echo "<tr><td colspan='15'>No data found</td></tr>";
}

// ปิดตาราง HTML
echo "</table>";

// ปิดการเชื่อมต่อฐานข้อมูล
$conn->close(); 
?>