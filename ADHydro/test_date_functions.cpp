#include "all.h"
#include <assert.h>

// The original versions of the date conversion functions that I cleaned up.
// UGLY!  I'm keeping them to compare against the new versions.
double greg_2_jul(
long year, 
long mon, 
long day, 
long h, 
long mi, 
double se)
{
    long m = mon, d = day, y = year;
    long c, ya, j;
    double seconds = h * 3600.0 + mi * 60 + se;

    if (m > 2)
        m -= 3;
    else {
        m += 9;
        --y;
    }
    c = y / 100L;
    ya = y - (100L * c);
    j = (146097L * c) / 4L + (1461L * ya) / 4L + (153L * m + 2L) / 5L + d + 1721119L;
    if (seconds < 12 * 3600.0) {
        j--;
        seconds += 12.0 * 3600.0;
    }
    else {
        seconds = seconds - 12.0 * 3600.0;
    }
    return (j + (seconds / 3600.0) / 24.0);
}

void calc_date(double jd, long *y, long *m, long *d, long *h, long *mi,
               double *sec)
{
    static int ret[4];

    long j;
    double tmp; 
    double frac;

    j=(long)jd;
    frac = jd - j;

    if (frac >= 0.5) {
        frac = frac - 0.5;
        j++;
    }
    else {
        frac = frac + 0.5;
    }

    ret[3] = (j + 1L) % 7L;
    j -= 1721119L;
    *y = (4L * j - 1L) / 146097L;
    j = 4L * j - 1L - 146097L * *y;
    *d = j / 4L;
    j = (4L * *d + 3L) / 1461L;
    *d = 4L * *d + 3L - 1461L * j;
    *d = (*d + 4L) / 4L;
    *m = (5L * *d - 3L) / 153L;
    *d = 5L * *d - 3 - 153L * *m;
    *d = (*d + 5L) / 5L;
    *y = 100L * *y + j;
    if (*m < 10)
        *m += 3;
    else {
        *m -= 9;
        *y=*y+1; /* Invalid use: *y++. Modified by Tony */
    }

    /* if (*m < 3) *y++; */
    /* incorrectly repeated the above if-else statement. Deleted by Tony.*/

    tmp = 3600.0 * (frac * 24.0);
    *h = (long) (tmp / 3600.0);
    tmp = tmp - *h * 3600.0;
    *mi = (long) (tmp / 60.0);
    *sec = tmp - *mi * 60.0;
}

int main(void)
{
  long   year;
  long   month;
  long   day;
  long   hour;
  long   minute;
  double second;
  double julian;
  long   yearCheck;
  long   monthCheck;
  long   dayCheck;
  long   hourCheck;
  long   minuteCheck;
  double secondCheck;
  
  // Test case 1: Noon, Jan 1, 4713 BCE, the reference date for Julian date.
  // Because there was no year zero 4713 BCE is year -4712.
  // These functions don't work properly for negative years, but at least check that the new code gets the same answer as the old code.
  assert(39.0 == gregorianToJulian(-4712, 1, 1, 12, 0, 0.0));
  assert(39.0 == greg_2_jul(       -4712, 1, 1, 12, 0, 0.0));
  julianToGregorian(0.0, &year, &month, &day, &hour, &minute, &second);
  assert(-4712 == year && 0 == month && -1 == day && 12 == hour && 0 == minute && 0.0 == second);
  calc_date(        0.0, &year, &month, &day, &hour, &minute, &second);
  assert(-4712 == year && 0 == month && -1 == day && 12 == hour && 0 == minute && 0.0 == second);
  
  // Test case 2: Beginning midnight, Jan 1, 1, the reference date for Rata Die.
  assert(1721425.5 == gregorianToJulian(1, 1, 1, 0, 0, 0.0));
  assert(1721425.5 == greg_2_jul(       1, 1, 1, 0, 0, 0.0));
  julianToGregorian(1721425.5, &year, &month, &day, &hour, &minute, &second);
  assert(1 == year && 1 == month && 1 == day && 0 == hour && 0 == minute && 0.0 == second);
  calc_date(        1721425.5, &year, &month, &day, &hour, &minute, &second);
  assert(1 == year && 1 == month && 1 == day && 0 == hour && 0 == minute && 0.0 == second);
  
  // Test case 3: Beginning midnight, Oct 15, 1582, the reference date for Lillian date.
  assert(2299160.5 == gregorianToJulian(1582, 10, 15, 0, 0, 0.0));
  assert(2299160.5 == greg_2_jul(       1582, 10, 15, 0, 0, 0.0));
  julianToGregorian(2299160.5, &year, &month, &day, &hour, &minute, &second);
  assert(1582 == year && 10 == month && 15 == day && 0 == hour && 0 == minute && 0.0 == second);
  calc_date(        2299160.5, &year, &month, &day, &hour, &minute, &second);
  assert(1582 == year && 10 == month && 15 == day && 0 == hour && 0 == minute && 0.0 == second);
  
  // Test case 4: Beginning midnight, Jan 1, 1601, the reference date for ANSI date.
  assert(2305813.5 == gregorianToJulian(1601, 1, 1, 0, 0, 0.0));
  assert(2305813.5 == greg_2_jul(       1601, 1, 1, 0, 0, 0.0));
  julianToGregorian(2305813.5, &year, &month, &day, &hour, &minute, &second);
  assert(1601 == year && 1 == month && 1 == day && 0 == hour && 0 == minute && 0.0 == second);
  calc_date(        2305813.5, &year, &month, &day, &hour, &minute, &second);
  assert(1601 == year && 1 == month && 1 == day && 0 == hour && 0 == minute && 0.0 == second);
  
  // Test case 5: Noon, Nov 16, 1858, the reference date for reduced Julian date.
  assert(2400000.0 == gregorianToJulian(1858, 11, 16, 12, 0, 0.0));
  assert(2400000.0 == greg_2_jul(       1858, 11, 16, 12, 0, 0.0));
  julianToGregorian(2400000.0, &year, &month, &day, &hour, &minute, &second);
  assert(1858 == year && 11 == month && 16 == day && 12 == hour && 0 == minute && 0.0 == second);
  calc_date(        2400000.0, &year, &month, &day, &hour, &minute, &second);
  assert(1858 == year && 11 == month && 16 == day && 12 == hour && 0 == minute && 0.0 == second);
  
  // Test case 6: Noon, Dec 31, 1899, the reference date for Dublin Julian date.
  assert(2415020.0 == gregorianToJulian(1899, 12, 31, 12, 0, 0.0));
  assert(2415020.0 == greg_2_jul(       1899, 12, 31, 12, 0, 0.0));
  julianToGregorian(2415020.0, &year, &month, &day, &hour, &minute, &second);
  assert(1899 == year && 12 == month && 31 == day && 12 == hour && 0 == minute && 0.0 == second);
  calc_date(        2415020.0, &year, &month, &day, &hour, &minute, &second);
  assert(1899 == year && 12 == month && 31 == day && 12 == hour && 0 == minute && 0.0 == second);
  
  // Test case 7: Beginning midnight, Jan 1, 1970, the reference date for Unix time.
  assert(2440587.5 == gregorianToJulian(1970, 1, 1, 0, 0, 0.0));
  assert(2440587.5 == greg_2_jul(       1970, 1, 1, 0, 0, 0.0));
  julianToGregorian(2440587.5, &year, &month, &day, &hour, &minute, &second);
  assert(1970 == year && 1 == month && 1 == day && 0 == hour && 0 == minute && 0.0 == second);
  calc_date(        2440587.5, &year, &month, &day, &hour, &minute, &second);
  assert(1970 == year && 1 == month && 1 == day && 0 == hour && 0 == minute && 0.0 == second);
  
  // Test case 8: Random dates between 1900 and 2100.  Check old and new functions against each other.
  for (julian = 2415020.0; julian < 2488068.0; julian += (double)rand() / RAND_MAX)
    {
      julianToGregorian(julian, &year,      &month,      &day,      &hour,      &minute,      &second);
      calc_date(        julian, &yearCheck, &monthCheck, &dayCheck, &hourCheck, &minuteCheck, &secondCheck);
      assert(year == yearCheck && month == monthCheck && day == dayCheck && hour == hourCheck && minute == minuteCheck && second == secondCheck);
      assert(julian == gregorianToJulian(year, month, day, hour, minute, second));
      assert(julian == greg_2_jul(       year, month, day, hour, minute, second));
    }
  
  return 0;
}
