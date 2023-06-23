>Lo primero que hay que hacer es descargarse el repositorio de github del siguiente link: https://github.com/benjamindelasoie/pop3-protos 
>Luego, en la terminal dirigirse hasta el directorio donde se haya creado la carpeta y dentro de ella realizar el comando ‘cd nonblock’.
>Ejecutar el comando ‘make clean’ y ‘CC=<compiler> make’ dónde compiler indica el compilador a utilizar.
>Finalmente, para ejecutar el servidor ejecutar el comando ‘./pop3 <mail_directory> <port>’ dónde mail_directory indica el directorio donde se van a encontrar las carpetas de los mails, estas tienen que ser de la siguiente manera:’mail_directory/<user>/cur’. Y <port> el número de puerto para escuchar. El default sera el 1080 y para el monitoreo el 1800.
>Además a se va a necesitar un archivo ‘users.txt’ listando los usuarios y contraseñas con el siguiente formato: ‘user,pass’
>Para ejecutar la aplicacion cliente se debe entrar a la carpeta "client" y ejecutar el comando ‘make clean’ y ‘CC=<compiler> make’. Luego se debe ejecutar el comnado ./monnitor seguido de la direccion IP y el puerto designado.
