#ifndef PDF_REPORT_HEADER
#define PDF_REPORT_HEADER

// Construye un SELECT (sin ';\n' final) que devuelve UNA SOLA fila con un
// documento PDF 1.4 (hoja A4 horizontal, fuentes base 14 Helvetica-Bold y
// Courier) armado enteramente en SQL puro -- sin extensiones ni librerias
// externas. 
// Recibe el rango del periodo ya resuelto a fechas ISO YYYY-MM-DD ("from" y
// "to" deben apuntar a buffers de al menos 11 bytes). Devuelve un string SQL
// en heap; el caller debe liberarlo con free().
char * buildPdfReportSelect(const char * from, const char * to);

#endif
