#include "VerifyFormat.h"
#include "corelib.h"

bool VerifyImportDateTimeString(const stringT &time_str, time_t &t)
{
  //  String format must be "yyyy/mm/dd hh:mm:ss"
  //                        "0123456789012345678"

  stringT xtime_str;
  const int month_lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const int idigits[14] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18};
  const int ndigits = 14;
  int yyyy, mon, dd, hh, min, ss, nscanned;

  t = (time_t)-1;

  if (time_str.length() != 19)
    return false;

  // Validate time_str
  if (time_str.substr(4,1) != _S("/") ||
      time_str.substr(7,1) != _S("/") ||
      time_str.substr(10,1) != _S(" ") ||
      time_str.substr(13,1) != _S(":") ||
      time_str.substr(16,1) != _S(":"))
    return false;

  for (int i = 0;  i < ndigits; i++)
    if (!isdigit(time_str[idigits[i]]))
      return false;

  // Since white space is ignored with _stscanf, first verify that there are no invalid '#' characters
  // Then take copy of the string and replace all blanks by '#' (should only be 1)
  if (time_str.find(TCHAR('#')) != stringT::npos)
    return false;

  xtime_str = time_str;
  if (Replace(xtime_str, TCHAR(' '), TCHAR('#')) != 1)
    return false;

#if _MSC_VER >= 1400
  nscanned = _stscanf_s(xtime_str.c_str(), _T("%4d/%2d/%2d#%2d:%2d:%2d"),
                        &yyyy, &mon, &dd, &hh, &min, &ss);
#else
  nscanned = _stscanf(xtime_str.c_str(), _T("%4d/%2d/%2d#%2d:%2d:%2d"),
                      &yyyy, &mon, &dd, &hh, &min, &ss);
#endif

  if (nscanned != 6)
    return false;

  // Built-in obsolesence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon == 2 && (yyyy % 4) == 0) {
    // Feb and a leap year
    if (dd > 29)
      return false;
  } else {
    // Either (Not Feb) or (Is Feb but not a leap-year)
    if (dd > month_lengths[mon - 1])
      return false;
  }

  if ((hh < 0 || hh > 23) ||
      (min < 0 || min > 59) ||
      (ss < 0 || ss > 59))
    return false;

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with CTime constructor after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = (time_t)0;
    return true;
  }

  const CTime ct(yyyy, mon, dd, hh, min, ss, -1);

  t = (time_t)ct.GetTime();

  return true;
}

bool VerifyASCDateTimeString(const stringT &time_str, time_t &t)
{
  //  String format must be "ddd MMM dd hh:mm:ss yyyy"
  //                        "012345678901234567890123"

  const int month_lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const stringT str_months = _T("JanFebMarAprMayJunJulAugSepOctNovDec");
  const stringT str_days = _T("SunMonTueWedThuFriSat");
  stringT xtime_str;
  TCHAR cmonth[4], cdayofweek[4];
  const int idigits[12] = {8, 9, 11, 12, 14, 15, 17, 18, 20, 21, 22, 23};
  const int ndigits = 12;
  stringT::size_type iMON, iDOW, nscanned;
  int yyyy, mon, dd, hh, min, ss;

  cmonth[3] = cdayofweek[3] = TCHAR('\0');

  t = (time_t)-1;

  if (time_str.length() != 24)
    return false;

  // Validate time_str
  if (time_str.substr(13,1) != _S(":") ||
      time_str.substr(16,1) != _S(":"))
    return false;

  for (int i = 0; i < ndigits; i++)
    if (!isdigit(time_str[idigits[i]]))
      return false;

  // Since white space is ignored with _stscanf, first verify that there are no invalid '#' characters
  // Then take copy of the string and replace all blanks by '#' (should be 4)
  if (time_str.find(TCHAR('#')) != stringT::npos)
    return false;

  xtime_str = time_str;
  if (Replace(xtime_str, TCHAR(' '), TCHAR('#')) != 4)
    return false;

#if _MSC_VER >= 1400
  nscanned = _stscanf_s(xtime_str.c_str(), _T("%3c#%3c#%2d#%2d:%2d:%2d#%4d"),
                        cdayofweek, sizeof(cdayofweek), cmonth, sizeof(cmonth),
                        &dd, &hh, &min, &ss, &yyyy);
#else
  nscanned = _stscanf(xtime_str.c_str(), _T("%3c#%3c#%2d#%2d:%2d:%2d#%4d"),
                      cdayofweek, cmonth, &dd, &hh, &min, &ss, &yyyy);
#endif

  if (nscanned != 7)
    return false;

  iMON = str_months.find(cmonth);
  if (iMON == stringT::npos)
    return false;

  mon = (iMON / 3) + 1;

  // Built-in obsolesence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon == 2 && (yyyy % 4) == 0) {
    // Feb and a leap year
    if (dd > 29)
      return false;
  } else {
    // Either (Not Feb) or (Is Feb but not a leap-year)
    if (dd > month_lengths[mon - 1])
      return false;
  }

  if ((hh < 0 || hh > 23) ||
      (min < 0 || min > 59) ||
      (ss < 0 || ss > 59))
    return false;

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with CTime constructor after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = (time_t)0;
    return true;
  }

  const CTime ct(yyyy, mon, dd, hh, min, ss, -1);

  iDOW = str_days.find(cdayofweek);
  if (iDOW == stringT::npos)
    return false;

  iDOW = (iDOW / 3) + 1;
  if (iDOW != stringT::size_type(ct.GetDayOfWeek()))
    return false;

  t = (time_t)ct.GetTime();

  return true;
}

bool VerifyXMLDateTimeString(const stringT &time_str, time_t &t)
{
  //  String format must be "yyyy-mm-ddThh:mm:ss"
  //                        "0123456789012345678"

  stringT xtime_str;
  const int month_lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const int idigits[14] = {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17, 18};
  const int ndigits = 14;
  int yyyy, mon, dd, hh, min, ss, nscanned;

  t = (time_t)-1;

  if (time_str.length() != 19)
    return false;

  // Validate time_str
  if (time_str.substr(4,1) != _S("-") ||
      time_str.substr(7,1) != _S("-") ||
      time_str.substr(10,1) != _S("T") ||
      time_str.substr(13,1) != _S(":") ||
      time_str.substr(16,1) != _S(":"))
    return false;

  for (int i = 0; i < ndigits; i++) {
    if (!isdigit(time_str[idigits[i]]))
      return false;
  }

  // Since white space is ignored with _stscanf, first verify that there are no invalid '#' characters
  // and no blanks.  Replace '-' & 'T' by '#'.
  if (time_str.find(TCHAR('#')) != stringT::npos)
    return false;
  if (time_str.find(TCHAR(' ')) != stringT::npos)
    return false;

  xtime_str = time_str;
  if (Replace(xtime_str, TCHAR('-'), TCHAR('#')) != 2)
    return false;
  if (Replace(xtime_str, TCHAR('T'), TCHAR('#')) != 1)
    return false;

#if _MSC_VER >= 1400
  nscanned = _stscanf_s(xtime_str.c_str(), _T("%4d#%2d#%2d#%2d:%2d:%2d"),
                        &yyyy, &mon, &dd, &hh, &min, &ss);
#else
  nscanned = _stscanf(xtime_str.c_str(), _T("%4d#%2d#%2d#%2d:%2d:%2d"),
                      &yyyy, &mon, &dd, &hh, &min, &ss);
#endif

  if (nscanned != 6)
    return false;

  // Built-in obsolesence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon == 2 && (yyyy % 4) == 0) {
    // Feb and a leap year
    if (dd > 29)
      return false;
  } else {
    // Either (Not Feb) or (Is Feb but not a leap-year)
    if (dd > month_lengths[mon - 1])
      return false;
  }

  if ((hh < 0 || hh > 23) ||
      (min < 0 || min > 59) ||
      (ss < 0 || ss > 59))
    return false;

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with CTime constructor after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = (time_t)0;
    return true;
  }

  const CTime ct(yyyy, mon, dd, hh, min, ss, -1);

  t = (time_t)ct.GetTime();

  return true;
}

bool VerifyXMLDateString(const stringT &time_str, time_t &t)
{
  //  String format must be "yyyy-mm-dd"
  //                        "0123456789"

  stringT xtime_str;
  const int month_lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const int idigits[14] = {0, 1, 2, 3, 5, 6, 8, 9};
  const int ndigits = 8;
  int yyyy, mon, dd, nscanned;

  t = (time_t)-1;

  if (time_str.length() != 10)
    return false;

  // Validate time_str
  if (time_str.substr(4,1) != _S("-") ||
      time_str.substr(7,1) != _S("-"))
    return false;

  for (int i = 0; i < ndigits; i++) {
    if (!isdigit(time_str[idigits[i]]))
      return false;
  }

  // Since white space is ignored with _stscanf, first verify that there are no invalid '#' characters
  // and no blanks.  Replace '-' by '#'.
  if (time_str.find(TCHAR('#')) != stringT::npos)
    return false;
  if (time_str.find(TCHAR(' ')) != stringT::npos)
    return false;

  xtime_str = time_str;
  if (Replace(xtime_str, TCHAR('-'), TCHAR('#')) != 2)
    return false;

#if _MSC_VER >= 1400
  nscanned = _stscanf_s(xtime_str.c_str(), _T("%4d#%2d#%2d"),
                        &yyyy, &mon, &dd);
#else
  nscanned = _stscanf(xtime_str.c_str(), _T("%4d#%2d#%2d"),
                      &yyyy, &mon, &dd);
#endif

  if (nscanned != 3)
    return false;

  // Built-in obsolesence for pwsafe in 2038?
  if (yyyy < 1970 || yyyy > 2038)
    return false;

  if ((mon < 1 || mon > 12) || (dd < 1))
    return false;

  if (mon == 2 && (yyyy % 4) == 0) {
    // Feb and a leap year
    if (dd > 29)
      return false;
  } else {
    // Either (Not Feb) or (Is Feb but not a leap-year)
    if (dd > month_lengths[mon - 1])
      return false;
  }

  // Accept 01/01/1970 as a special 'unset' value, otherwise there can be
  // issues with CTime constructor after apply daylight savings offset.
  if (yyyy == 1970 && mon == 1 && dd == 1) {
    t = (time_t)0;
    return true;
  }

  const CTime ct(yyyy, mon, dd, 0, 0, 0, -1);

  t = (time_t)ct.GetTime();

  return true;
}

int VerifyImportPWHistoryString(const StringX &PWHistory,
                                StringX &newPWHistory, stringT &strErrors)
{
  // Format is (! == mandatory blank, unless at the end of the record):
  //    sxx00
  // or
  //    sxxnn!yyyy/mm/dd!hh:mm:ss!llll!pppp...pppp!yyyy/mm/dd!hh:mm:ss!llll!pppp...pppp!.........
  // Note:
  //    !yyyy/mm/dd!hh:mm:ss! may be !1970-01-01 00:00:00! meaning unknown

  StringX tmp, pwh;
  stringT buffer;
  int ipwlen, s = -1, m = -1, n = -1;
  int rc = PWH_OK;
  time_t t;

  newPWHistory = _T("");
  strErrors = _T("");

  pwh = PWHistory;
  int len = pwh.length();
  int pwleft = len;

  if (pwleft == 0)
    return PWH_OK;

  if (pwleft < 5) {
    rc = PWH_INVALID_HDR;
    goto exit;
  }

  const TCHAR *lpszPWHistory = pwh.c_str();

#if _MSC_VER >= 1400
  int iread = _stscanf_s(lpszPWHistory, _T("%01d%02x%02x"), &s, &m, &n);
#else
  int iread = _stscanf(lpszPWHistory, _T("%01d%02x%02x"), &s, &m, &n);
#endif
  if (iread != 3) {
    rc = PWH_INVALID_HDR;
    goto relbuf;
  }

  if (s != 0 && s != 1) {
    rc = PWH_INVALID_STATUS;
    goto relbuf;
  }

  if (n > m) {
    rc = PWH_INVALID_NUM;
    goto relbuf;
  }

  lpszPWHistory += 5;
  pwleft -= 5;

  if (pwleft == 0 && s == 0 && m == 0 && n == 0) {
    rc = PWH_IGNORE;
    goto relbuf;
  }

  Format(buffer, _T("%01d%02x%02x"), s, m, n);
  newPWHistory = buffer.c_str();

  for (int i = 0; i < n; i++) {
    if (pwleft < 26) {  //  blank + date(10) + blank + time(8) + blank + pw_length(4) + blank
      rc = PWH_TOO_SHORT;
      goto relbuf;
    }

    if (lpszPWHistory[0] != _T(' ')) {
      rc = PWH_INVALID_CHARACTER;
      goto relbuf;
    }

    lpszPWHistory += 1;
    pwleft -= 1;

    tmp = StringX(lpszPWHistory, 19);

    if (tmp.substr(0, 10) == _T("1970-01-01"))
      t = 0;
    else {
      if (!VerifyImportDateTimeString(tmp.c_str(), t)) {
        rc = PWH_INVALID_DATETIME;
        goto relbuf;
      }
    }

    lpszPWHistory += 19;
    pwleft -= 19;

    if (lpszPWHistory[0] != _T(' ')) {
      rc = PWH_INVALID_CHARACTER;
      goto relbuf;
    }

    lpszPWHistory += 1;
    pwleft -= 1;

#if _MSC_VER >= 1400
    iread = _stscanf_s(lpszPWHistory, _T("%04x"), &ipwlen);
#else
    iread = _stscanf(lpszPWHistory, _T("%04x"), &ipwlen);
#endif
    if (iread != 1) {
      rc = PWH_INVALID_PSWD_LENGTH;
      goto relbuf;
    }

    lpszPWHistory += 4;
    pwleft -= 4;

    if (lpszPWHistory[0] != _T(' ')) {
      rc = PWH_INVALID_CHARACTER;
      goto relbuf;
    }

    lpszPWHistory += 1;
    pwleft -= 1;

    if (pwleft < ipwlen) {
      rc = PWH_INVALID_PSWD_LENGTH;
      goto relbuf;
    }

    tmp = StringX(lpszPWHistory, ipwlen);
    Format(buffer, _T("%08x%04x%s"), (long) t, ipwlen, tmp);
    newPWHistory += buffer.c_str();
    buffer.clear();
    lpszPWHistory += ipwlen;
    pwleft -= ipwlen;
  }

  if (pwleft > 0)
    rc = PWH_TOO_LONG;

 relbuf: // pwh.ReleaseBuffer(); - obsoleted by move to StringX

 exit: Format(buffer, IDSC_PWHERROR, len - pwleft + 1);
  stringT temp;
  switch (rc) {
  case PWH_OK:
  case PWH_IGNORE:
    temp.clear();
    buffer.clear();
    break;
  case PWH_INVALID_HDR:
    Format(temp, IDSC_INVALIDHEADER, PWHistory);
    break;
  case PWH_INVALID_STATUS:
    Format(temp, IDSC_INVALIDPWHSTATUS, s);
    break;
  case PWH_INVALID_NUM:
    Format(temp, IDSC_INVALIDNUMOLDPW, n, m);
    break;
  case PWH_INVALID_DATETIME:
    LoadAString(temp, IDSC_INVALIDDATETIME);
    break;
  case PWH_INVALID_PSWD_LENGTH:
    LoadAString(temp, IDSC_INVALIDPWLENGTH);
    break;
  case PWH_TOO_SHORT:
    LoadAString(temp, IDSC_FIELDTOOSHORT);
    break;
  case PWH_TOO_LONG:
    LoadAString(temp, IDSC_FIELDTOOLONG);
    break;
  case PWH_INVALID_CHARACTER:
    LoadAString(temp, IDSC_INVALIDSEPARATER);
    break;
  default:
    ASSERT(0);
  }
  strErrors = buffer + temp;
  if (rc != PWH_OK)
    newPWHistory = _T("");

  return rc;
}
