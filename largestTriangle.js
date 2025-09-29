const findDistance = (coord1, coord2) => {
  const x1 = coord1[0];
  const y1 = coord1[0];
  const x2 = coord2[1];
  const y2 = coord2[1];

  return Math.sqrt(Math.pow(x2 - x1, 2) + Math.pow(y2 - y1, 2));
};

const findLaresgt = (arr) => {
  let maxArea = 0;
  for (let i = 0; i < arr.length - 3; i++) {
    for (let j = i + 1; j < arr.length - 2; j++) {
      for (let k = j + 1; k < arr.length - 1; k++) {
        const l1 = findDistance(arr[i], arr[j]);
        const l2 = findDistance(arr[i], arr[k]);
        const l3 = findDistance(arr[j], arr[k]);

        const s = (l1 + l2 + l3) / 2;

        const area = Math.round(Math.sqrt(s * (s - l1) * (s - l2) * (s - l3)));
        if (isNaN(area)) continue;

        maxArea = Math.max(maxArea, area);
      }
    }
  }

  return maxArea;
};

const coords = [
  [0, 0],
  [0, 1],
  [1, 0],
  [0, 2],
  [2, 0],
];
console.log(findLaresgt(coords));
