# Check whether Docker is running
docker version *>$null
if (-not $?) {
  Write-Error "Docker is not running"
  exit 1
}

# Define the project parameters
$project = Split-Path -Path $PWD -Leaf
$gcc_version = 'latest'
$container = "$project-gcc-dev-$gcc_version"

# Define the volume mapping
$host_dir = $PWD.Path
$container_dir = "/usr/src/${project}"

# Ensure that the container is available and configured, and relatively up-to-date
($created = docker container inspect -f '{{ .Created }}' "$container") *>$null
if (-not $?) {
  $create = $true
} elseif ([DateTime]$created -lt (Get-Date).AddMonths(-1)) {
  $create = $true

  # Remove the old container
  Write-Output "Removing container `"$container`""
  docker container rm -f "$container"
} else {
  $create = $false
}
if ($create) {
  # Ensure the gcc image is available
  $gcc = "gcc:$gcc_version"
  docker pull $gcc

  # Create the container with correct mapping
  Write-Output "Creating container `"$container`""
  docker run -d --name "$container" -v "${host_dir}:${container_dir}" $gcc sh -c 'trap "exit 0" TERM; tail -f /dev/null & wait'

  # Install dependencies
  Write-Output "Installing dependencies: cmake, valgrind, unbuffer"
  docker exec -it "$container" bash -c 'apt-get -qq update && apt-get -qq install cmake valgrind expect'

  # Stop container
  docker stop "$container" *>$null
}

try {
  # Start container
  docker start $container *>$null

  # Run full-test.sh inside the container
  Write-Output "Executing test script"
  docker exec -w "$container_dir" "$container" ./full-test.sh $args
} finally {
  # Stop container
  docker stop "$container" *>$null
}
