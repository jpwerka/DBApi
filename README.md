DBApi
=====

Database ODBC API


Esta é uma biblioteca escrita em C++ que tem por objetivo efetuar a conexão com banco de dados via ODBC abstraindo as chamadas a API ODBC pura, porém não limitando, visto que ela pode retornar o manipuladores (Handles) dos objetos utilizados pela API ODBC pura.

É totalmente orientada a objetos facilitando a manipulação dos dados utilizando métodos intuitivos para executar cada ação.
Possui estrutura hierárquica que faz com que cada objeto de dados possua seus conjuntos de parâmetros ou campos de acordo com a necessidade. Exemplo: Uma consulta pode possuir um conjunto de parâmetros que comportam cada parâmetro e esta consulta pode retornar um conjunto de campos que comportam cada campo, facilitando assim o acesso as informações.

Como um dos objetivos desta biblioteca é ser utilizada com uma extensão (Windows DLL), ela utiliza o conceito de interfaces para instanciar os objetos, ou seja, todas as classes herdam de uma classe totalmente abstrata que será utilizada apenas para manipular as chamadas dos métodos de cada classe em particular.

Levando em consideração esta condição, nenhuma exceção é disparada diretamente, porém podem ser capturadas de forma simples por um objeto estático que sempre irá conter a ultima exceção ocorrida.

Porém, nada impede que os objetos sejam compilados incorporados a sua aplicação dispensando o uso de uma biblioteca externa para tal funcionalidade. Neste caso, pode-se utilizar de forma direta dos objetos.

#Compilação

As únicas dependências da biblioteca são das classes STD padrão C++, portanto, esta biblioteca pode ser portável. Pode ser, pois não testei em outras plataformas diferentes do Windows :(.

Ela pode ser compilada utilizando o Visual Studio ou o MingW com gcc para Windows.
Se alguém quiser colaborar para testar a mesma num ambiente Linux ou Unix, fico grato e estou disposto ajudar no que for preciso :).

Esta biblioteca permite a manipulação e alocação de memória para os campos da consulta de forma padrão, automática, ou pode ser controlada pela aplicação através de ponteiros de memória para as variáveis que receberão os valores dos campos.
