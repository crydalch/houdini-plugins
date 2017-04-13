# Houdini version environment management
htool() {
    if [ "$#" -ne 1 ]; then
        eval $(python3 $HOME/bin/htool.py "$@")
    else
        python3 $HOME/bin/htool.py $1
    fi
}

# Create a Fossil-based Houdini Project
newproj() {
    if [ ! "$#" -eq 0 ] || [ $1 = "-h" ] || [ $1 = "--help" ]; then
        echo "Pass a project name like this: hrpoj PROJECTNAME"
    else
        mkdir $1
        cd $1
        fossil new --template $HOME/Templates/houproj.fossil --date-override "$(date +"%Y-%m-%d %H:%M:%S")" $1.fossil
        echo "(Ignore the intitial password, it has been made blank)"
        fossil sqlite3 -R $1.fossil "UPDATE user SET pw='' WHERE login='$(echo $USER)';"
        fossil sqlite3 -R $1.fossil "REPLACE INTO config VALUES ('project-name','$(echo $1)','')"
        fossil open $1.fossil
    fi
}
