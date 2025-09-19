Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/focal64"
  
  config.vm.network "forwarded_port", guest: 8080, host: 8080, auto_correct: true
  config.ssh.forward_x11 = true
  # Optional: set VM resources
  config.vm.provider "virtualbox" do |vb|
    vb.memory = "2048"
    vb.cpus = 2
  end

  # Optional: synced folder
  config.vm.synced_folder ".", "/home/vagrant/project"

  # Provisioning: install build tools and drogons deps
  config.vm.provision "shell", inline: <<-SHELL
    sudo apt update
    sudo apt install -y build-essential cmake git libssl-dev uuid-dev libboost-all-dev libsqlite3-dev zlib1g-dev libbrotli-dev libpq-dev libmysqlclient-dev libhiredis-dev doxygen

  SHELL
end
