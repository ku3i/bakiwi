# SO(2) network, testing implementation
# written by Matthias Kubisch
# March 5th 2019
# kubisch@informatik.hu-berlin.de
#
# The theoretical basis can be found in this paper:
# 'SO(2)-Networks as Neural Oscillators' by Pasemann, Hild and Zahedi


phi = pi/50.0;  # pi/10 .. pi/50

k = (1.0 + 1.5*phi); # 1 + 1pi ... 1 + 2pi

s = cos(phi) * k;
r = sin(phi) * k;

W = [s,r; -r,s]; # weight matrix

x = [.001; .0] # initialized state vector

N = 1000; # max time steps
X = zeros(2,N); # container for plotting
T = [1:N]; # time

for n = T
  x = tanh(W*x);
  X(:,n) = x;
end

subplot(221);
plot(X(1,:),X(2,:));

subplot(212);
plot(T,X);
