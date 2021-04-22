Map config = [
  aptlyConfig: 'release-aptly-config',
  uploadJob: 'contactless/wb-releases/master',
  defaultTargets: 'wb5 wb6'
]

pipeline {
  agent {
    label 'devenv'
  }
  parameters {
    string(name: 'TARGETS', defaultValue: config.defaultTargets, description: 'space-separated list')
    booleanParam(name: 'ADD_VERSION_SUFFIX', defaultValue: true, description: 'for dev branches only')
    booleanParam(name: 'UPLOAD_TO_POOL', defaultValue: true,
                 description: 'works only with ADD_VERSION_SUFFIX to keep staging clean')
    booleanParam(name: 'REPLACE_RELEASE', defaultValue: false,
                 description: 'delete existing Github release before publishing')
    string(name: 'WBDEV_IMAGE', defaultValue: '', description: 'docker image to use as devenv')
  }
  environment {
    WBDEV_INSTALL_DEPS = 'yes'
    WBDEV_BUILD_METHOD = 'sbuild'
    WBDEV_TARGET = 'wb6'
    PROJECT_SUBDIR = 'project'
    RESULT_SUBDIR = 'result'
  }
  options {
    checkoutToSubdirectory('project')
  }
  stages {
    stage('Cleanup workspace') {
      steps {
        cleanWs deleteDirs: true, patterns: [[pattern: "$RESULT_SUBDIR", type: 'INCLUDE']]
      }
    }
    stage('Determine version prefix') {
      when {
        not { anyOf {
          branch 'master'
          branch 'main'
          branch 'release/*'
        }}
        expression {
          params.ADD_VERSION_SUFFIX
        }
      }

      steps {
        dir("$PROJECT_SUBDIR") {
          script {
            // FIXME: need to get master ref in some other way
            sh 'git config remote.origin.fetch "+refs/heads/*:refs/remotes/origin/*" && git fetch --all'

            def versionSuffix = wb.makeVersionSuffixFromBranch()
            env.SBUILD_ARGS = "--append-to-version='${versionSuffix}' --maintainer='Robot'"
          }
        }
      }
    }

    stage('Setup builds') {
      steps {
        script {
          def targets = params.TARGETS.split(' ')
          def jobs = [:]

          for (target in targets) {
            def currentTarget = target
            jobs["build ${currentTarget}"] = {
              stage("Build ${currentTarget}") {
                dir("$PROJECT_SUBDIR") {
                  sh "printenv | sort"
                  sh "wbdev root printenv | sort"
                  sh "WBDEV_TARGET=${currentTarget} wbdev cdeb ${env.SBUILD_ARGS?:''} -j`nproc`"
                }
              }
            }
          }

          parallel jobs
        }
      }
      post {
        always {
          sh 'mkdir -p $RESULT_SUBDIR && (find . -maxdepth 1 -type f -exec mv "{}" $RESULT_SUBDIR \\; )'
          dir("$PROJECT_SUBDIR") {
            sh 'wbdev root chown -R jenkins:jenkins .'
          }
        }
        success {
          archiveArtifacts artifacts: "$RESULT_SUBDIR/*.deb"
        }
      }
    }

    stage('Add packages to pool') {
      when { expression {
        params.UPLOAD_TO_POOL && params.ADD_VERSION_SUFFIX
      }}

      environment {
        APTLY_CONFIG = credentials("${config.aptlyConfig}")
      }

      steps {
        sh 'wbci-repo -c $APTLY_CONFIG add-debs -f -d "jenkins:$JOB_NAME.$BUILD_NUMBER" $RESULT_SUBDIR/*.deb'
      }
    }
  
    stage('Upload via wb-releases') {
      when { expression {
        params.UPLOAD_TO_POOL && params.ADD_VERSION_SUFFIX
      }}

      steps {
        build job: config.uploadJob, wait: true, parameters: [booleanParam(name: 'FORCE_OVERWRITE', value: true)]
      }
    }

    stage('Github release') {
      when { anyOf {
        branch 'master'
        branch 'main'
        branch 'release/*'
      }}

      environment {
        GITHUB_TOKEN = credentials('fixme-webconn-github-token')
      }

      steps {
        dir("$PROJECT_SUBDIR") {
          sh 'wbci-git -v -t $GITHUB_TOKEN_PSW publish-release ' +
             "${params.REPLACE_RELEASE ? '-f ' : ''}" + '../$RESULT_SUBDIR/*.deb'
        }
      }
    }
  }
}
