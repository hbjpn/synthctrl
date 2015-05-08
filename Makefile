synthctrl: synthctrl.o
	g++ $^ -lasound -o $@

.cpp.o:
	g++ -c -lasound $^

clean:
	rm -f *.o synthctrl
