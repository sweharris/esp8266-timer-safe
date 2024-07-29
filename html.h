static const String change_ap_html = R"EOF(
<html>
<body>

<form method=post enctype="multipart/form-data">
To set the WiFi network details, enter here:
<br>
WiFi SSID: <input name=ssid size=40>
<br>
WiFi Password: <input name=password size=40>
<br>
Safe name: <input name=safename size=40 value="##safename##">
<br>
GPIO pin to trigger solenoid (default 12): <input name=pin size=2 value="##pin##">
<br>
For reference these are the pins on a NodeMCU 12E board:
<blockquote>
<pre>
GPIO16 == D0
GPIO5  == D1
GPIO4  == D2 (pin on ESP8266/Relay board)
GPIO0  == D3
GPIO2  == D4
GPIO14 == D5
GPIO12 == D6 (the default)
GPIO13 == D7
GPIO15 == D8
</pre>
</blockquote>
<br>
<input type=submit value="Set WiFi" name=setwifi>
<hr>
If the change is accepted, the safe will reboot after 5 seconds.
<hr>
<p>
&nbsp;
<p>
&nbsp;
<font size=-3>Software version: ##VERSION##</font>
</form>
</body>
</html>
)EOF";

static const String change_auth_html = R"EOF(
<html>
<body>

<form method=post action=safe/ enctype="multipart/form-data">
To set the user name and password needed to access the safe:
<br>
Safe Username: <input name=username size=40>
<br>
Safe Password: <input name=password size=40>
<br>
<input type=submit value="Set Auth Details" name=setauth>
<hr>
If the change is accepted, you will need to login again.
</form>
</body>
</html>
)EOF";

static const String index_html = R"EOF(
<!DOCTYPE html>
<html>

<head>
<title>Safe</title>
<link rel="shortcut icon" href="data:image/x-icon;base64,AAABAAEAEA4AAAEAIADgAwAAFgAAACgAAAAQAAAAHAAAAAEAIAAAAAAAgAMAAMMOAADDDgAAAAAAAAAAAAD+/v7///////j4+P/29vb///////////////////////////////////////7+/v/19fX/+/v7///////9/f3//////6Kiov9ISEj/T09P/4mJif+Ghob/hoaG/4WFhf+Ghob/hoaG/4eHh/+CgoL/RERE/1VVVf++vr7//////8rKyv9ycnL/2dnZ/6qqqv+np6f/pqam/6SkpP+pqan/qKio/6SkpP+mpqb/p6en/7Kysv/R0dH/aGho//Hx8f+9vb3/mZmZ/4CAgP9zc3P/jo6O/4+Pj/+ampr/gICA/4WFhf+bm5v/ioqK/5OTk/9hYWH/nJyc/4mJif/l5eX/v7+//5SUlP+Ghob/5eXl///////y8vL/jIyM/3Fxcf9xcXH/oaGh//7+/v//////wMDA/5KSkv+IiIj/5eXl/76+vv+UlJT/h4eH/+Pj4///////Tk5O/6Kiov/a2tr/5ubm/3Jycv93d3f//////7q6uv+UlJT/iIiI/+Xl5f++vr7/l5eX/3p6ev/l5eX/wMDA/4+Pj//R0dH/Y2Nj/2pqav/m5ub/cHBw//Hx8f/Gxsb/kpKS/4iIiP/l5eX/vb29/56env9XV1f/paWl/6+vr/+goKD/iYmJ/9fX1/+lpaX/rKys/319ff/a2tr/y8vL/5KSkv+IiIj/5eXl/76+vv+VlZX/gYGB/+7u7v/Ly8v/gICA/+Li4v9jY2P/d3d3/+rq6v9oaGj/+vr6/8TExP+Tk5P/iIiI/+Xl5f++vr7/lZWV/4eHh//i4uL//////2JiYv+BgYH/3Nzc/97e3v9XV1f/j4+P//////+8vLz/lJSU/4iIiP/l5eX/v7+//5OTk/+FhYX/5eXl///////9/f3/oqKi/2ZmZv9ra2v/ubm5////////////wMDA/5GRkf+IiIj/5eXl/729vf+YmJj/kpKS/21tbf+Dg4P/f39//5CQkP+Kior/jIyM/46Ojv99fX3/hoaG/2JiYv+srKz/hYWF/+Xl5f/U1NT/aGho/8HBwf+lpaX/p6en/6ampv+kpKT/paWl/6SkpP+kpKT/p6en/6ampv+rq6v/tra2/2dnZ//39/f//////7S0tP9/f3//jY2N/4uLi/+Li4v/i4uL/4uLi/+Li4v/i4uL/4uLi/+Li4v/jIyM/39/f//Nzc3//////wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" />
</head>

<frameset rows = "8%,*">
  <frame name = "top" src = "top_frame.html" />

  <frameset cols = "30%,70%">
    <frame name = "menu" src = "menu_frame.html" />
    <frame name = "main" src = "status.html" />
  </frameset>

  <noframes>
     <body>Your browser does not support frames.</body>
  </noframes>

</frameset>

</html>
)EOF";

static const String menu_frame_html = R"EOF(
<html>
<head>
  <base target="main">
  <title>Safe menu</title>
</head>
<body>
<center><h2>Menu</h2>
<p>
<a href=status.html>Status</a>
<hr>
<form method=post action=safe/ enctype="multipart/form-data">
Open safe door:<br>
<select name="duration">
  <option value="5">5 seconds</option>
  <option value="10">10 seconds</option>
  <option value="20">20 seconds</option>
  <option value="30">30 seconds</option>
  <option value="60">60 seconds</option>
</select>&nbsp;&nbsp;
<input type=submit value="Open Safe" name=open>
</form>
<hr>
<a href="change_auth.html">Change Safe Authentication Details</a>
<hr>
<a href="change_ap.html">Change Safe Connection Details</a>
<br>
<hr>
<a href="enable_update">Enable Safe Network Update</a>
<br>
<a href="disable_update">Disable Safe Network Update</a>
<br>
<hr>
</center>
</form>
</body>
</html>
)EOF";

static const String status_html = R"EOF(
<html>
<head>
  <script>
  function GET(url)
  {
    var x = new XMLHttpRequest();
    x.open("GET", url, false);
    x.send();
    if (x.status === 200)
    {
        return x.responseText;
    }
  };

  // From https://stackoverflow.com/a/52387803
  function secondsToDhms(seconds)
  {
    seconds = Number(seconds);
    var d = Math.floor(seconds / (3600*24));
    var h = Math.floor(seconds % (3600*24) / 3600);
    var m = Math.floor(seconds % 3600 / 60);
    var s = Math.floor(seconds % 60);

    var dDisplay = d > 0 ? d + (d == 1 ? " day, " : " days, ") : "";
    var hDisplay = h > 0 ? h + (h == 1 ? " hour, " : " hours, ") : "";
    var mDisplay = m > 0 ? m + (m == 1 ? " minute, " : " minutes, ") : "";
    var sDisplay = s > 0 ? s + (s == 1 ? " second" : " seconds") : "";
    return (dDisplay + hDisplay + mDisplay + sDisplay).replace(/,\s*$/, "");
  }

  function checknum(val, field)
  {
    if (!Number.isInteger(+val))
    { 
      alert(field + " is not a number");
      return false;
    }
    return true;
  }

  function calctime()
  {
    var ld=Number(document.time.ld.value); if (!checknum(ld,"Min Days")) { return false };
    var lh=Number(document.time.lh.value); if (!checknum(lh,"Min Hours")) { return false };
    var lm=Number(document.time.lm.value); if (!checknum(lm,"Min Mins")) { return false };
    var ls=Number(document.time.ls.value); if (!checknum(ls,"Min Secs")) { return false };
    var ladd=((ld*24+lh)*60+lm)*60+ls;
    document.time.laddtime.value=ladd;
    console.log('low',ld,lh,lm,ls,ladd);
  
    var hd=Number(document.time.hd.value); if (!checknum(hd,"Max Days")) { return false };
    var hh=Number(document.time.hh.value); if (!checknum(hh,"Max Hours")) { return false };
    var hm=Number(document.time.hm.value); if (!checknum(hm,"Max Mins")) { return false };
    var hs=Number(document.time.hs.value); if (!checknum(hs,"Max Secs")) { return false };
    var hadd=((hd*24+hh)*60+hm)*60+hs;
    if (hadd==0) { hadd=ladd; }
    document.time.haddtime.value=hadd;
    console.log('high',hd,hh,hm,hs,hadd);

    if (hadd==0 && ladd==0) { alert("No times entered"); return false; }
    return true;
  }

  var t=GET("safe/?status=1");

  const date=new Date();
  const end=new Date(date.getTime() + t*1000)

  </script>
</head>
<body>
<center>
<h1>The safe is
<script>if (t==0) { document.write("not locked and may be opened") } else { document.write("locked"); }</script>
</h1>
<hr>
<table>
<tr><td align=right>The current time is:</td><td><script>document.write(date)</script></td></tr>
<script>if (t>0)
{
  document.write("<tr><td align=right>The expected unlock time is:</td><td>" + end + "</td></tr>");
  document.write("<tr><td align=right>Expected time remaining:</td><td>" + secondsToDhms(t) + "</td></tr>");
} else if (t==-1)
{
  document.write("<tr><td align=right>Expected time remaining:</td><td><b>Hidden</b></td></td>");
}
</script>
</table>
</center>

<hr>
<center>
  <h2>Add time</h2>
  <form name=time action=safe/ method=post onsubmit="return calctime()" enctype="multipart/form-data">
    Min Days: <input name=ld size=3>
    Hours: <input name=lh size=3>
    Minutes: <input name=lm size=3>
    Seconds: <input name=ls size=3>
<p>
    Max Days: <input name=hd size=3>
    Hours: <input name=hh size=3>
    Minutes: <input name=hm size=3>
    Seconds: <input name=hs size=3>

    <input type=hidden name=laddtime>
    <input type=hidden name=haddtime>
<p>
(Only enter Max values if you want the safe to pick a random period between these dates)
<p>
    Hide timer? <input type=checkbox name=hide>
  <input type=submit value="Add Time">
</form>
</center>

</body>
</html>
)EOF";

static const String top_frame_html = R"EOF(
<html>
<body>
<center><h1>Safe lock controls</h1></center>
</body>
</html>
)EOF";

