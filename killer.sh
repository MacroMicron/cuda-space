sleep 20 
kill `ps aux | grep Space | grep out | sed s/"mr-pi     "// | sed s/" .*"//`
