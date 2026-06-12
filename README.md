[![✗](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/facundokrens65049/TPE_TLA/releases)

[![✗](https://github.com/facundokrens65049/TPE_TLA/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/facundokrens65049/TPE_TLA/actions/workflows/pipeline.yaml)

# TPE_TLA — Compilador de un DSL de Finanzas Personales

Trabajo Práctico Especial de **Autómatas, Teoría de Lenguajes y Compiladores** (ITBA).

Este proyecto implementa un **lenguaje específico de dominio (DSL)** para administrar
**finanzas personales** de un único usuario, y un compilador —escrito en C con Flex y
Bison— que **valida semánticamente** los programas y **genera un script SQL para
PostgreSQL** que persiste, consulta y mantiene la información financiera.

* [Dominio](#dominio)
* [El lenguaje](#el-lenguaje)
* [Qué hace el compilador](#qué-hace-el-compilador)
* [Requisitos](#requisitos)
* [Comandos](#comandos)
* [Configuración](#configuración)
* [Tests](#tests)
* [CI/CD](#cicd)
* [Equipo](#equipo)
* [Licencia](#licencia)

## Dominio

Muchas personas administran sus finanzas de forma manual, dispersando la información
entre apps bancarias, notas y planillas. Eso dificulta mantener un historial
consistente, corregir errores de carga, proyectar obligaciones futuras (compras en
cuotas, suscripciones) y obtener resúmenes claros.

El DSL abstrae ese problema: permite **describir operaciones financieras mediante
sentencias de alto nivel**, sin que el usuario deba interactuar directamente con SQL
ni conocer el esquema relacional subyacente. La solución se apoya en una base de datos
**PostgreSQL** persistente, donde cada operación recibe un identificador único
autogenerado que luego se usa para editarla, eliminarla o finalizarla.

Alcance acotado a un único usuario: sin integración con bancos, sin sincronización con
cuentas reales y sin cotizaciones en tiempo real. Las divisas se registran como contexto
de las operaciones (no hay conversión automática). Las categorías no se declaran
previamente: cuando aparecen, se almacenan **normalizadas** (minúscula y sin tildes).

## El lenguaje

Construcciones principales:

| Construcción | Descripción |
| :----------- | :---------- |
| `divisa <X>` | Fija la divisa por defecto para las operaciones siguientes. |
| `ingreso <monto> [categoria ...] [fecha ...] [descripcion ...]` | Registra un ingreso. |
| `gasto <monto> [cuotas <n>] [categoria ...] [fecha ...] [descripcion ...]` | Registra un gasto, opcionalmente en cuotas (se derivan las obligaciones futuras). |
| `suscripcion <monto> <frecuencia> [categoria ...] desde <fecha> [hasta <fecha>] [descripcion ...]` | Registra un gasto recurrente. |
| `consultar (desde <fecha> hasta <fecha> \| <período>)` | Consulta operaciones por rango o período predefinido. |
| `reporte (texto plano \| html \| pdf) (desde <fecha> hasta <fecha> \| <período>)` | Genera un reporte en el formato y período indicados. |
| `editar <id> ...` | Modifica una operación referenciándola por su id. |
| `eliminar <id>` | Elimina una operación por su id. |
| `finalizar <id>` | Finaliza una suscripción o un plan de cuotas, conservando el historial. |

Detalles:

* **Fechas:** absolutas en formato `dd-mm-yyyy`, o relativas (`ayer`, `hoy`, `maniana`).
* **Períodos predefinidos:** `semanal`, `mensual`, `anual` (toman la fecha actual como cierre).
* **Palabras reservadas:** no son *case sensitive* e ignoran tildes (`Descripcion` ≡ `descripción`).
* **Identificadores** (categorías, divisas): empiezan con letra (con o sin tilde / `ñ`) y pueden contener letras, dígitos, `_` y `-` (p. ej. `gastos-fijos`, `USD-BLUE`). El guion no puede ir al inicio.
* **Descripción:** todo lo que sigue a `descripcion` es texto libre, truncado a 80 caracteres.
* **Montos:** números sin signo, con dos decimales útiles. Como separador decimal se acepta `.` o `,`, **a lo sumo uno** por monto (`100.50` ≡ `100,50`; mezclar (`100,50.5`) es error léxico). Si se escriben más de dos decimales, se **truncan** (no se redondea) a los dos primeros, ya que la base persiste como `NUMERIC(15,2)`: `100.555` → `100.55`, `99.999` → `99.99`. Admiten sufijos multiplicadores `K` (×10³), `M` (×10⁶) y `B` (×10⁹), pegados o separados por espacios/tabs (no por saltos de línea), encadenables y *case-insensitive*: `5K` ≡ `5 k`, `1,5KB` ≡ `1.5 k b` ≡ 1.5·10¹². Internamente se trabaja en centavos sobre `int64` (~9.22·10¹⁸); si la cuenta se desborda, el lexema se rechaza.
* **Identificadores numéricos:** `cuotas N` y los ids referenciados por `editar`/`eliminar`/`finalizar` deben ser enteros (sin parte decimal). `editar 5.5 ...` o `cuotas 6.5` se rechazan en análisis semántico.

Ejemplos:

```text
divisa ARS
gasto 180K cuotas 6 categoria electrodomesticos descripcion heladera nueva
suscripcion 7999.99 mensual categoria streaming desde 01-03-2026 descripcion plataforma de peliculas
ingreso 1.5 M categoria sueldo
reporte html desde 01-01-2026 hasta 31-03-2026
```

## Qué hace el compilador

1. **Frontend (Flex + Bison):** análisis léxico y sintáctico; construye el AST.
2. **Análisis semántico:** valida estáticamente que
   * los montos de `gasto`/`ingreso`/`suscripcion`/`editar monto` sean **positivos**;
   * la cantidad de `cuotas` sea **un entero ≥ 1** (sin parte decimal);
   * los ids de `editar`/`eliminar`/`finalizar` sean **enteros** (sin parte decimal);
   * las fechas literales sean **válidas según el calendario** (incluyendo años bisiestos);
   * los rangos de `consultar`/`reporte` sean **consistentes** (`desde ≤ hasta`);
   * una `suscripcion` no tenga fecha de finalización **anterior** a la de inicio.
   * Las validaciones referenciales (existencia de un id, que `finalizar` apunte a una
     suscripción/cuotas) se delegan a **runtime** contra la base, ya que los ids los
     genera PostgreSQL.
3. **Generación de código:** sobre un AST validado emite a `stdout` un **script SQL para
   PostgreSQL** (DDL idempotente + `INSERT`/`UPDATE`/`DELETE`/`SELECT` por sentencia,
   derivación de cuotas, reportes en texto plano y HTML).

El compilador comunica el resultado por **código de salida**: `0` si el programa es
aceptado, distinto de `0` si es rechazado (léxico, sintáctico o semántico).

## Requisitos

Todo se ejecuta dentro de un contenedor, así que sólo hace falta:

* [Docker](https://www.docker.com/) (con Docker Compose).

El contenedor ya provee la cadena de build: **Flex, Bison, GCC, CMake y Make**. El
proyecto compila con `-fsanitize=address` y `-O3`, de modo que el binario debe quedar
libre de fugas y errores de memoria.

Para **ejecutar** el SQL generado se necesita, adicionalmente, una instancia de
**PostgreSQL** (el compilador sólo emite el script; no se conecta a la base).

## Comandos

Levantar un contenedor efímero listo para trabajar:

```bash
docker compose run --rm compiler
```

Buildear (o rebuildear) el compilador completo:

```bash
docker compose run --rm compiler bash src/main/bash/build.sh
```

Compilar un programa (emite el SQL por `stdout`):

```bash
docker compose run --rm compiler bash src/main/bash/run.sh <programa>
```

donde `<programa>` es la ruta al archivo de entrada.

## Configuración

Variables de entorno para controlar el comportamiento de la aplicación:

| Nombre                | Default | Descripción                                                                                                                                                          |
| :-------------------- | :-----: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ENVIRONMENT`         | `Local` | Entorno activo. Disponibles: `Local`, `Development` y `Production`.                                                                                                   |
| `LOG_IGNORED_LEXEMES` | `true`  | Si es `true`, loguea a nivel `DEBUGGING` los lexemas ignorados encontrados por Flex. Poner en `false` para quitarlos de la salida.                                    |
| `LOGGING_LEVEL`       | `ALL`   | Nivel mínimo a loguear. De menor a mayor: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR` y `CRITICAL`.                                                         |

_Docker Compose_ también puede leer estas variables desde un archivo `.env` (ver `compose.yaml`).

## Tests

Los tests viven en `src/test/c/{accept,reject}/` (un programa por archivo) y se evalúan
por **código de salida**: los de `accept/` deben aceptarse (`0`), los de `reject/` deben
rechazarse (`≠0`).

```bash
docker compose run --rm compiler bash src/main/bash/test.sh
```

## CI/CD

El repositorio incluye un workflow de _GitHub Actions_ (`.github/workflows/pipeline.yaml`)
que instala las dependencias, buildea el compilador y corre la suite de tests en cada
push y pull request.

## Equipo

| Nombre              | Legajo  |
| :------------------ | :-----: |
| Carolina Castelnuovo | 65.539 |
| Dalila Orbaj         | 65.471 |
| Facundo Krens        | 65.049 |
| Florencia Cecotto    | 65.068 |

## Licencia

Distribuido bajo licencia MIT. Ver [LICENSE.md](LICENSE.md).
