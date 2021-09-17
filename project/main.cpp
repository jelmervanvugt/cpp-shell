extern int shell(bool prompt);

int main(int argc, char** argv) {
	bool showPrompt = argc == 1;
	return shell(showPrompt);
}
