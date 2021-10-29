make:
	g++ -pthread convolution.cc -o convolution

run:
	./convolution test_matrix 1 5

clean:
	rm convolution