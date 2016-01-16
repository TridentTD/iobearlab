1. สร้าง file ชื่อ nodemcu.txt โดยมี format ดังนี้

ชื่อ node;ประเภท node;password

ประเภท node จะมี NODE_PRIMARY กับ NODE_SECONDARY ให้เลือก 2 ประเภท
ชื่อ node กำหนดเองต่างหาก
password เอาไว้กำหนดตอนจะทำเครือข่ายกันคนภายนอกเข้า..ตอนนี้ให้เป็นว่างๆไว้

ตย.
nodeMain;NODE_PRIMARY;

จากนั้น แฟรช ขึ้นสู่ SPIFFS ของ node


2.โปรแกรม arduino ide ทำการ แฟรชแอพลงสู่พื้นที่ sketch
 