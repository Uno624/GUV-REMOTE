
<!DOCTYPE html>
<html lang="th"> <!-- กำหนดว่าเป็น HTML ใช้ภาษาไทย -->
<head>
    <meta charset="UTF-8"> <!-- กำหนด encoding เป็น UTF-8 เพื่อรองรับภาษาไทย -->
    <title>รถสำรวจภาพดิน Dashboard</title> <!-- ชื่อหน้าเว็บที่จะแสดงบนแท็บเบราว์เซอร์ -->
    <link rel="stylesheet" href="style.css"> <!-- เชื่อมไฟล์ CSS สำหรับตกแต่งหน้าเว็บ -->
</head>
<body>

    <!-- ส่วนหัวของหน้าเว็บ -->
    <header>
        <h1>Soil Survey Dashboard</h1> <!-- หัวข้อหลักของ Dashboard -->
    </header>
    <!-- ส่วนแสดงข้อมูลเซนเซอร์ -->
    <section class="sensor-data">
        <h2>Sensor Data</h2>
        <?php include 'show.php'; ?> <!-- ดึงข้อมูลจากไฟล์ show.php มาแสดงเป็นตาราง -->
    </section>
</body>
</html>

