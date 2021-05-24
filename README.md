![What you should see when running the application](https://github.com/andreacasalino/Easy-Factor-Graph-GUI/blob/master/Example.png)

This graphic user interface is inteded to **create**, **inspect**, **import** or **export** undirected graphical models.
The application is completely cross platform thanks to [this](https://github.com/yhirose/cpp-httplib) C++ http server implementation.
The application is made of 2 big components:

* **Frontend**, an **html** script represented by **EFG-GUI.html**
* **Backend**, a **C++** application that can be compiled using [Cmake](https://cmake.org) which wraps [Easy-Factor-Graph](https://github.com/andreacasalino/Easy-Factor-Graph), a library I wrote for handling undirected graphical models.

**Compile**:

* initialize the git submodule with the following command (from root) 
  * `git submodule update --init --recursive`

* compile the [Cmake](https://cmake.org) project an run the **INSTALL** command
  * after that, the backend application named **EFG-GUI** will be appear in the installation folder, under bin

**Run**:

* got to the installation folder and then inside bin.
* run the application named **EFG-GUI**
* open in your favourite browser the script **EFG-GUI.html**
* have fun with the GUI :)
