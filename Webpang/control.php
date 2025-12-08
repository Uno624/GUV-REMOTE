<?php
// รับค่าคำสั่งจากฟอร์ม
if(isset($_POST['command'])) {
    $command = $_POST['command'];

    // ตรงนี้คุณสามารถเชื่อมต่อไปยังระบบควบคุมรถจริง เช่น ส่งไป Arduino หรือ API
    // ตอนนี้ทำเป็นตัวอย่างง่าย ๆ แค่แสดงข้อความ
    if($command == "forward") {
        echo "รถเดินหน้า";
    } elseif($command == "backward") {
        echo "รถถอยหลัง";
    } elseif($command == "left") {
        echo "รถหันซ้าย";
    } elseif($command == "right") {
        echo "รถหันขวา";
    } else {
        echo "ไม่รู้จักคำสั่ง";
    }
} else {
    echo "No command received";
}
?>
