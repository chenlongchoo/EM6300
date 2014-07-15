function toggleView(a)
{
	var s = document.getElementById(a);
	if( s.style.display == 'none' ) s.style.display = 'inline';
	else s.style.display = 'none';
}

function isNumber(n) {
  return !isNaN(parseFloat(n)) && isFinite(n);
}