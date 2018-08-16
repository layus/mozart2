#!Groovy

pipeline {

    agent any

    stages {

        stage ('Generate sources') {
            options { 
                checkoutToSubdirectory('mozart2')
            }
            steps {
                sh 'mkdir -p llvm/tools/clang llvm-install llvm-build'
                dir ('llvm') {
                    #git clone --branch release_40 https://github.com/llvm-mirror/llvm || true
                    curl http://releases.llvm.org/3.8.1/llvm-3.8.1.src.tar.xz | tar xJvf - --strip-components=1
                    
                    dir ('tools/clang') {
                        #git clone --branch release_40 https://github.com/llvm-mirror/clang || true
                        sh 'curl http://releases.llvm.org/3.8.1/cfe-3.8.1.src.tar.xz | tar xJvf - --strip-components=1'
                        sh 'export PREFIX=$(pwd)'
                    }
                }
                dir ('llvm-build') {
                    sh 'cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=`pwd`/../llvm-install ../llvm'
                    sh 'make -j4'
                    sh 'make install -j4'
                }
                dir ('llvm-install') {
                    sh 'export PATH=`pwd`/bin:$PATH'
                }
                checkout scm
                dir('build') {
                    sh 'cmake -DCMAKE_BUILD_TYPE=Release -DMOZART_BOOST_USE_STATIC_LIBS=False -DCMAKE_PROGRAM_PATH=$BIN -DCMAKE_PREFIX_PATH=$PREFIX $SOURCES'
                    sh 'make dist VERBOSE=1 -j4'
                }
            }
        }


        stage ('Build') {
            steps {
                echo 'lol'
            }
        }

    }
}
