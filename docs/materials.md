# Materials

Materials in TPM are represented by an `unsigned long, ulong` value. This was
done to reduce complexity by having a material index, and referencing that. With
this implementation, the `SDF()` function is permitted to generate materials at
render time, which would not be possible with a material map being indexed. This
does mean that each ray could result in a different material as specified by the
`SDF()` function.

## Format

The first four bits of the integer represent the material type, according to the
table below. Each material type interprets the remaining bits of the material
differently to account for the different fields required for each material type.

| Binary | Hex | Type        |
|--------|-----|-------------|
| `0000` | `0` | NONE        |
| `0001` | `1` | LIGHT       |
| `0010` | `2` | GLASS       |
| `0011` | `3` | MATTE       |
| `0100` | `4` | METAL       |
| `0101` | `5` | MIRROR      |
| `0110` | `6` | PLASTIC     |
| `0111` | `7` | TRANSLUCENT |

0123456789 0123456789 0123456789 0123456789 0123456789 0123456789 0123
TTTT

FFFF FFFF FFFF FFFF
TTRR GGBB

### Light

| Field | N | Bits | Min | Max | Precision | Size |
|-------|---|------|-----|-----|-----------|------|
| Type  | 1 | 4    | 0   | 16  | 1.0       | 4.0  |
| Ke    | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| Total |   |      |     |     | 0.0       | 28.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$3,7=Sum(1,7:2,7)

### Glass

| Field      | N | Bits | Min | Max | Precision | Size |
|------------|---|------|-----|-----|-----------|------|
| Type       | 1 | 4    | 0   | 16  | 1.0       | 4.0  |
| Kr         | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| Kt         | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| uRoughness | 1 | 8    |     |     | 0.0       | 8.0  |
| vRoughness | 1 | 8    |     |     | 0.0       | 8.0  |
| index      | 1 | 8    | 1   | 3   | 0.007812  | 8.0  |
| Total      |   |      |     |     | 0.0       | 76.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$7,7=Sum(1,7:6,7)

### Matte

| Field | N | Bits | Min | Max | Precision   | Size |
|-------|---|------|-----|-----|-------------|------|
| Type  | 1 | 4    | 0   | 16  | 1.0         | 4.0  |
| Kd    | 3 | 8    | 0   | 1   | 0.003906    | 24.0 |
| sigma | 1 | 32   | 0   | 90  | 2.095476e-8 | 32.0 |
| Total |   |      |     |     | 0.0         | 60.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$4,7=Sum(1,7:3,7)

### Metal

| Field      | N | Bits | Min | Max | Precision | Size |
|------------|---|------|-----|-----|-----------|------|
| Type       | 1 | 4    | 0   | 16  | 1.0       | 4.0  |
| eta        | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| K          | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| roughness  | 1 | 8    |     |     | 0.0       | 8.0  |
| uRoughness | 1 | 8    |     |     | 0.0       | 8.0  |
| vRoughness | 1 | 8    |     |     | 0.0       | 8.0  |
| Total      |   |      |     |     | 0.0       | 76.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$7,7=Sum(1,7:6,7)

### Mirror

| Field | N | Bits | Min | Max | Precision | Size |
|-------|---|------|-----|-----|-----------|------|
| Type  | 1 | 4    | 0   | 16  | 1.0       | 4.0  |
| Kr    | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| Total |   |      |     |     | 0.0       | 28.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$3,7=Sum(1,7:2,7)

### Plastic

| Field     | N | Bits | Min | Max | Precision    | Size |
|-----------|---|------|-----|-----|--------------|------|
| Type      | 1 | 4    | 0   | 16  | 1.0          | 4.0  |
| Kd        | 3 | 8    | 0   | 1   | 0.003906     | 24.0 |
| roughness | 1 | 32   | 0   | 1   | 2.328306e-10 | 32.0 |
| Total     |   |      |     |     | 0.0          | 60.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$4,7=Sum(1,7:3,7)

### Translucent

| Field     | N | Bits | Min | Max | Precision | Size |
|-----------|---|------|-----|-----|-----------|------|
| Type      | 1 | 4    | 0   | 16  | 1.0       | 4.0  |
| Kd        | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| Kr        | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| Kt        | 3 | 8    | 0   | 1   | 0.003906  | 24.0 |
| roughness | 1 | 8    | 0   | 1   | 0.003906  | 8.0  |
| Total     |   |      |     |     | 0.0       | 84.0 |
> tmf: $6=($5-$4)/pow(2, $3);$7=$2*$3;$6,7=Sum(1,7:5,7)
