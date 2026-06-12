#ifndef TEXT_REPORT_HEADER
#define TEXT_REPORT_HEADER

// Construye un SELECT (sin ';\n' final) que devuelve UNA SOLA fila con el
// reporte formateado como tabla ASCII de ancho fijo: cabecera + separador +
// filas con wrap multi-linea (categoria y descripcion no se truncan, se
// reparten en lineas debajo) + balance neto por divisa al pie. Comparte el
// modelo de filas y la cabecera con el PDF (mismas COL_*_WIDTH).
//
// Recibe el rango del periodo ya resuelto a fechas ISO YYYY-MM-DD ("from" y
// "to" deben apuntar a buffers de al menos 11 bytes). Devuelve un string SQL
// en heap; el caller debe liberarlo con free().
char * buildTextReportSelect(const char * from, const char * to);

#endif
