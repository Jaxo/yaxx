/* qpi
|
| This script computes PI to (quite) the precision 'precision'.
*/

qpi:

PARSE ARG precision .
IF precision="" THEN DO
   SAY "Precision? (less or equal to 1000, pls)"
   PULL precision
   END

NUMERIC DIGITS precision
convergenceSpeed = 14
k1 = 13591409
k2 = 545140134
k3 = 640320

s = 1
t = k1
k3_power3 = k3*k3*k3
DO n=1 BY 1 TO precision % convergenceSpeed
   s = -s *,
       (6*n) * (6*n-1) * (6*n-2) * (6*n-3) * (6*n-4) * (6*n-5),
       /,
       ((3*n) * (3*n-1) * (3*n-2) * n * n * n * k3_power3)
   t = t + s * (k1 + n*k2)
END
sqrt_k3 = 800;
DO N=1 by 1 TO 10
   sqrt_k3 = sqrt_k3 - (((sqrt_k3 * sqrt_k3) - k3) / (sqrt_k3 * 2))
END
Say (k3 * sqrt_k3) / 12 / t
