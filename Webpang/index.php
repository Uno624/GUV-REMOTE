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

    <!-- ส่วน Control Panel สำหรับควบคุมรถ -->
    <section class="control-panel">
        <h2>Control Panel</h2>
        <!-- ฟอร์มส่งคำสั่งไปที่ control.php ด้วย method POST -->
        <form action="control.php" method="post">
            <!-- ปุ่มแต่ละปุ่มส่งค่า action ไปยัง control.php -->
            <button class="forward" name="action" value="forward">เดินหน้า</button>
            <button class="left" name="action" value="left">หันซ้าย</button>
            <button class="right" name="action" value="right">หันขวา</button>
            <button class="backward" name="action" value="backward">ถอยหลัง</button>
            <button class="stop" name="action" value="stop">หยุดนิ่ง</button>
        </form>
    </section>

    <!-- ส่วนแสดงข้อมูลเซนเซอร์ -->
    <section class="sensor-data">
        <h2>Sensor Data</h2>
        <?php include 'show.php'; ?> <!-- ดึงข้อมูลจากไฟล์ show.php มาแสดงเป็นตาราง -->
    </section>

    <!-- ส่วนแสดงวิดีโอ feed -->
    <section class="video-feed">
        <h2>Video Feed</h2>
        <video controls> <!-- แท็ก video สำหรับเล่นวิดีโอ -->
            <source src="video.mp4" type="video/mp4"> <!-- ไฟล์วิดีโอที่จะแสดง -->
            เบราว์เซอร์ของคุณไม่รองรับ video tag <!-- แสดง ข้อความ  ถ้าเบราว์เซอร์ไม่รองรับ -->
        </video>
    </section>
</body>
</html>
