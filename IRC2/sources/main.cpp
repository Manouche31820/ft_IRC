# include "../headers/main.hpp"

int	main( int argc, char **argv )
{
	std::cout << "Let's go !" << std::endl;

	if (argc > 3) { // a ajuster avant soutenance
		std::cerr << "Invalid argv" << std::endl;
		return (1);
	}
	int port;
	if (argc > 1) {
		std::istringstream	ss(argv[1]);
		ss >> port;
	}
	else
		port = 6667;
	std::string password;
	if (argc > 2)
		password = argv[2];
	else
		password = "test";
	Server s1("127.0.0.1", port, password); // 0.0.0.0 :   ecoute sur toutes les ip de ce reseau local // sur ce reseau y a que ahcene: 127..1

	s1.start();

	return (0);
}
